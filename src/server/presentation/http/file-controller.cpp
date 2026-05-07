#include "file-controller.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"

namespace {

using namespace domain::models;
namespace json = boost::json;

json::object toJson(const File& file) {
    return json::object{{"id", file.id},
                        {"fullLogicalName", file.fullLogicalName},
                        {"currentVersionId", file.currentVersionId},
                        {"maxVersionCount", file.maxVersionCount},
                        {"createdAt", file.createdAt},
                        {"createdBy", file.createdBy}};
}

json::array toJsonArray(const std::vector<File>& files) {
    json::array items;
    items.reserve(files.size());

    for (const auto& file : files) {
        items.emplace_back(toJson(file));
    }

    return items;
}

inline bool requireFileAcl(infrastructure::http::RouteContext& ctx, domain::services::FileAclService& fileAclService,
                           int64_t userId, int64_t fileId, domain::models::AclLevel requiredAclLevel) {
    auto& req = ctx.request();

    const auto aclResult = fileAclService.getUserAclLevel(userId, fileId);
    if (!aclResult) {
        ctx.reply(presentation::http::makeServiceErrorResponse(req, aclResult.error()));
        return false;
    }

    if (*aclResult < requiredAclLevel) {
        ctx.reply(presentation::http::makeServiceErrorResponse(req, domain::services::ServiceError::Forbidden));
        return false;
    }

    return true;
}

}  // namespace

namespace presentation::http {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

FileController::FileController(FileService& fileService, FileContentService& fileContentService,
                               FileAclService& fileAclService, AuthTokenStore& tokenStore,
                               ThreadPool& threadPool) noexcept
    : m_fileService(fileService),
      m_fileContentService(fileContentService),
      m_fileAclService(fileAclService),
      m_tokenStore(tokenStore),
      m_threadPool(threadPool) {}

FileController::FileController(FileService& fileService, FileContentService& fileContentService,
                               FileAclService& fileAclService, AuthTokenStore& tokenStore, ThreadPool& threadPool,
                               NotificationPublisher& notificationPublisher) noexcept
    : FileController(fileService, fileContentService, fileAclService, tokenStore, threadPool) {
    m_notificationPublisher = &notificationPublisher;
}

void FileController::registerRoutes(Router& router) {
    router.add(http::verb::post, "/api/files", [this](RouteContext& ctx) { handleCreate(ctx); });

    router.add(http::verb::get, "/api/files", [this](RouteContext& ctx) { handleGetAll(ctx); });

    router.add(http::verb::get, "/api/files/by-id", [this](RouteContext& ctx) { handleGetById(ctx); });

    router.add(http::verb::get, "/api/files/by-name", [this](RouteContext& ctx) { handleGetByLogicalName(ctx); });

    router.add(http::verb::put, "/api/files/rename", [this](RouteContext& ctx) { handleRename(ctx); });

    router.add(http::verb::delete_, "/api/files", [this](RouteContext& ctx) { handleRemove(ctx); });
}

void FileController::handleCreate(RouteContext& ctx) {
    auto& req = ctx.request();

    const auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto body = parseJsonObject(req.body());
    if (!body) {
        ctx.reply(makeBadRequestResponse(req, "invalid_json"));
        return;
    }

    const auto logicalName = getStringField(*body, "logicalName");
    if (!logicalName || logicalName->empty()) {
        ctx.reply(makeBadRequestResponse(req, "logicalName is required"));
        return;
    }

    uint32_t maxVersionCount = 10;
    const auto maxVersionCountField = getInt64Field(*body, "maxVersionCount");
    if (maxVersionCountField) {
        if (*maxVersionCountField <= 0) {
            ctx.reply(makeBadRequestResponse(req, "maxVersionCount must be positive"));
            return;
        }

        maxVersionCount = *maxVersionCountField;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, logicalName = *logicalName, createdByUser = *authenticatedUserId,
                 maxVersionCount]() -> infrastructure::http::Response {
        auto createdStorageResult = m_fileContentService.createNew();
        if (!createdStorageResult) {
            return makeServiceErrorResponse(version, keepAlive, createdStorageResult.error());
        }

        const uint64_t physicalPath = createdStorageResult->physicalPath;

        auto createFileResult = m_fileService.create(logicalName, createdByUser, physicalPath, maxVersionCount);

        if (!createFileResult) {
            auto cleanupResult = m_fileContentService.remove(physicalPath);
            (void)cleanupResult;

            return makeServiceErrorResponse(version, keepAlive, createFileResult.error());
        }

        auto fileResult = m_fileService.getById(*createFileResult);
        if (!fileResult) {
            return makeServiceErrorResponse(version, keepAlive, fileResult.error());
        }

        if (m_notificationPublisher) {
            m_notificationPublisher->fileCreated(fileResult->id, createdByUser, fileResult->currentVersionId,
                                                 fileResult->fullLogicalName);
        }

        return infrastructure::http::makeJsonResponse(http::status::created, serializeJson(toJson(*fileResult)),
                                                      version, keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileController::handleGetAll(RouteContext& ctx) {
    auto& req = ctx.request();

    auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, userId = *authenticatedUserId]() -> infrastructure::http::Response {
        auto result = m_fileService.getAll();
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        std::vector<File> visibleFiles;
        visibleFiles.reserve(result->size());

        for (const auto& file : *result) {
            auto aclResult = m_fileAclService.getUserAclLevel(userId, file.id);
            if (!aclResult) {
                return makeServiceErrorResponse(version, keepAlive, aclResult.error());
            }

            if (*aclResult >= AclLevel::READ_ONLY) {
                visibleFiles.emplace_back(file);
            }
        }

        return infrastructure::http::makeJsonResponse(
            http::status::ok, serializeJson(json::object{{"items", toJsonArray(visibleFiles)}}), version, keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileController::handleGetById(RouteContext& ctx) {
    auto& req = ctx.request();

    auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto fileIdText = getQueryParam(req.target(), "fileId");
    if (!fileIdText) {
        ctx.reply(makeBadRequestResponse(req, "fileId is required"));
        return;
    }

    const auto fileId = parseInt64(*fileIdText);
    if (!fileId) {
        ctx.reply(makeBadRequestResponse(req, "fileId must be int64"));
        return;
    }

    if (!requireFileAcl(ctx, m_fileAclService, *authenticatedUserId, *fileId, AclLevel::READ_ONLY)) {
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, fileId = *fileId]() -> infrastructure::http::Response {
        auto result = m_fileService.getById(fileId);
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        return infrastructure::http::makeJsonResponse(http::status::ok, serializeJson(toJson(*result)), version,
                                                      keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileController::handleGetByLogicalName(RouteContext& ctx) {
    auto& req = ctx.request();

    auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto logicalNameText = getQueryParam(req.target(), "logicalName");
    if (!logicalNameText || logicalNameText->empty()) {
        ctx.reply(makeBadRequestResponse(req, "logicalName is required"));
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, userId = *authenticatedUserId,
                 logicalName = std::string(*logicalNameText)]() -> infrastructure::http::Response {
        auto result = m_fileService.getByLogicalName(logicalName);
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        auto aclResult = m_fileAclService.getUserAclLevel(userId, result->id);
        if (!aclResult) {
            return makeServiceErrorResponse(version, keepAlive, aclResult.error());
        }

        if (*aclResult < AclLevel::READ_ONLY) {
            return makeServiceErrorResponse(version, keepAlive, domain::services::ServiceError::Forbidden);
        }

        return infrastructure::http::makeJsonResponse(http::status::ok, serializeJson(toJson(*result)), version,
                                                      keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileController::handleRename(RouteContext& ctx) {
    auto& req = ctx.request();

    auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto body = parseJsonObject(req.body());
    if (!body) {
        ctx.reply(makeBadRequestResponse(req, "invalid_json"));
        return;
    }

    const auto fileId = getInt64Field(*body, "fileId");
    const auto newLogicalName = getStringField(*body, "newLogicalName");

    if (!fileId || !newLogicalName || newLogicalName->empty()) {
        ctx.reply(makeBadRequestResponse(req, "fileId and newLogicalName are required"));
        return;
    }

    if (!requireFileAcl(ctx, m_fileAclService, *authenticatedUserId, *fileId, AclLevel::READ_WRITE)) {
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, fileId = *fileId,
                 newLogicalName = *newLogicalName]() -> infrastructure::http::Response {
        auto result = m_fileService.rename(fileId, newLogicalName);
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        auto fileResult = m_fileService.getById(fileId);
        if (!fileResult) {
            return makeServiceErrorResponse(version, keepAlive, fileResult.error());
        }
        return infrastructure::http::makeJsonResponse(http::status::ok, serializeJson(toJson(*fileResult)), version,
                                                      keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileController::handleRemove(RouteContext& ctx) {
    auto& req = ctx.request();

    auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto fileIdText = getQueryParam(req.target(), "fileId");
    if (!fileIdText) {
        ctx.reply(makeBadRequestResponse(req, "fileId is required"));
        return;
    }

    const auto fileId = parseInt64(*fileIdText);
    if (!fileId) {
        ctx.reply(makeBadRequestResponse(req, "fileId must be int64"));
        return;
    }

    if (!requireFileAcl(ctx, m_fileAclService, *authenticatedUserId, *fileId, AclLevel::READ_WRITE)) {
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, fileId = *fileId]() -> infrastructure::http::Response {
        auto result = m_fileService.remove(fileId);
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        return infrastructure::http::makeJsonResponse(http::status::ok, R"({"status":"ok"})", version, keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

}  // namespace presentation::http