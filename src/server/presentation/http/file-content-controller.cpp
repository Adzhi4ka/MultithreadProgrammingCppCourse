#include "file-content-controller.h"

#include <boost/beast/core/file.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"

namespace {

using namespace domain::models;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

json::object toJson(const FileVersion& fileVersion) {
    return json::object{{"id", fileVersion.id},
                        {"fileId", fileVersion.fileId},
                        {"version", fileVersion.version},
                        {"logicalNameSnapshot", fileVersion.logicalNameSnapshot},
                        {"physicalPathName", fileVersion.physicalPathName},
                        {"createdAt", fileVersion.createdAt}};
}

inline std::optional<int64_t> getRequiredInt64QueryParam(const infrastructure::http::Request& req,
                                                         std::string_view name,
                                                         infrastructure::http::RouteContext& ctx) {
    const auto text = presentation::http::getQueryParam(req.target(), name);
    if (!text) {
        const std::string error = std::string(name) + " is required";
        ctx.reply(presentation::http::makeBadRequestResponse(req, error));
        return std::nullopt;
    }

    const auto value = presentation::http::parseInt64(*text);
    if (!value) {
        const std::string error = std::string(name) + " must be int64";
        ctx.reply(presentation::http::makeBadRequestResponse(req, error));
        return std::nullopt;
    }

    return value;
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

infrastructure::http::Response makeFileBodyResponse(unsigned version, bool keepAlive, const FileVersion& fileVersion,
                                                    const domain::services::DownloadFileStorage& download) {
    infrastructure::http::FileResponse response{http::status::ok, version};

    beast::error_code ec;
    response.body().open(download.path.c_str(), beast::file_mode::read, ec);
    if (ec) {
        return presentation::http::makeServiceErrorResponse(version, keepAlive,
                                                            domain::services::ServiceError::InternalError);
    }

    response.set(http::field::content_type, "application/octet-stream");
    response.set("X-File-Id", std::to_string(fileVersion.fileId));
    response.set("X-File-Version-Id", std::to_string(fileVersion.id));
    response.set("X-File-Version-Number", std::to_string(fileVersion.version));
    response.content_length(download.size);
    response.keep_alive(keepAlive);

    return infrastructure::http::Response{std::move(response)};
}

}  // namespace

namespace presentation::http {

namespace http = boost::beast::http;

FileContentController::FileContentController(FileVersionService& fileVersionService,
                                             FileContentService& fileContentService, FileAclService& fileAclService,
                                             AuthTokenStore& tokenStore, ThreadPool& threadPool) noexcept
    : m_fileVersionService(fileVersionService),
      m_fileContentService(fileContentService),
      m_fileAclService(fileAclService),
      m_tokenStore(tokenStore),
      m_threadPool(threadPool) {}

void FileContentController::registerRoutes(Router& router) {
    router.add(http::verb::get, "/api/files/content", [this](RouteContext& ctx) { handleDownloadCurrentContent(ctx); });

    router.add(http::verb::put, "/api/files/content", [this](RouteContext& ctx) { handleUploadCurrentContent(ctx); });

    router.add(http::verb::get, "/api/file-versions/content",
               [this](RouteContext& ctx) { handleDownloadVersionContent(ctx); });
}

void FileContentController::handleDownloadCurrentContent(RouteContext& ctx) {
    auto& req = ctx.request();

    const auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto fileId = getRequiredInt64QueryParam(req, "fileId", ctx);
    if (!fileId) {
        return;
    }

    if (!requireFileAcl(ctx, m_fileAclService, *authenticatedUserId, *fileId, AclLevel::READ_ONLY)) {
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, fileId = *fileId]() -> infrastructure::http::Response {
        auto currentVersionResult = m_fileVersionService.getCurrentVersion(fileId);
        if (!currentVersionResult) {
            return makeServiceErrorResponse(version, keepAlive, currentVersionResult.error());
        }

        auto downloadResult = m_fileContentService.openDownload(currentVersionResult->physicalPathName);
        if (!downloadResult) {
            return makeServiceErrorResponse(version, keepAlive, downloadResult.error());
        }

        return makeFileBodyResponse(version, keepAlive, *currentVersionResult, *downloadResult);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileContentController::handleUploadCurrentContent(RouteContext& ctx) {
    auto& req = ctx.request();

    const auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto fileId = getRequiredInt64QueryParam(req, "fileId", ctx);
    if (!fileId) {
        return;
    }

    if (!requireFileAcl(ctx, m_fileAclService, *authenticatedUserId, *fileId, AclLevel::READ_WRITE)) {
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();
    std::string content = req.body();

    auto task = [this, version, keepAlive, fileId = *fileId,
                 content = std::move(content)]() -> infrastructure::http::Response {
        auto previousVersionResult = m_fileVersionService.getCurrentVersion(fileId);
        if (!previousVersionResult) {
            return makeServiceErrorResponse(version, keepAlive, previousVersionResult.error());
        }

        auto createdStorageResult = m_fileContentService.createNew();
        if (!createdStorageResult) {
            return makeServiceErrorResponse(version, keepAlive, createdStorageResult.error());
        }

        const uint64_t physicalPath = createdStorageResult->physicalPath;

        auto writeResult = m_fileContentService.writeAll(physicalPath, content);
        if (!writeResult) {
            auto cleanupResult = m_fileContentService.remove(physicalPath);
            (void)cleanupResult;

            return makeServiceErrorResponse(version, keepAlive, writeResult.error());
        }

        auto createVersionResult =
            m_fileVersionService.createNewVersion(fileId, previousVersionResult->logicalNameSnapshot, physicalPath);
        if (!createVersionResult) {
            auto cleanupResult = m_fileContentService.remove(physicalPath);
            (void)cleanupResult;

            return makeServiceErrorResponse(version, keepAlive, createVersionResult.error());
        }

        auto currentVersionResult = m_fileVersionService.getCurrentVersion(fileId);
        if (!currentVersionResult) {
            return makeServiceErrorResponse(version, keepAlive, currentVersionResult.error());
        }

        return infrastructure::http::makeJsonResponse(http::status::created,
                                                      serializeJson(toJson(*currentVersionResult)), version, keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void FileContentController::handleDownloadVersionContent(RouteContext& ctx) {
    auto& req = ctx.request();

    const auto authenticatedUserId = authenticateUserId(req, m_tokenStore);
    if (!authenticatedUserId) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto versionId = getRequiredInt64QueryParam(req, "versionId", ctx);
    if (!versionId) {
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();

    auto task = [this, version, keepAlive, userId = *authenticatedUserId,
                 versionId = *versionId]() -> infrastructure::http::Response {
        auto fileVersionResult = m_fileVersionService.getVersionById(versionId);
        if (!fileVersionResult) {
            return makeServiceErrorResponse(version, keepAlive, fileVersionResult.error());
        }

        const auto aclResult = m_fileAclService.getUserAclLevel(userId, fileVersionResult->fileId);
        if (!aclResult) {
            return makeServiceErrorResponse(version, keepAlive, aclResult.error());
        }

        if (*aclResult < AclLevel::READ_ONLY) {
            return makeServiceErrorResponse(version, keepAlive, domain::services::ServiceError::Forbidden);
        }

        auto downloadResult = m_fileContentService.openDownload(fileVersionResult->physicalPathName);
        if (!downloadResult) {
            return makeServiceErrorResponse(version, keepAlive, downloadResult.error());
        }

        return makeFileBodyResponse(version, keepAlive, *fileVersionResult, *downloadResult);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

}  // namespace presentation::http
