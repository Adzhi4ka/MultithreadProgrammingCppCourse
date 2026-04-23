#include "file-version-controller.h"

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

    using namespace domain::models;
    namespace json = boost::json;

    json::object toJson(const FileVersion& fileVersion) {
        return json::object{
            {"id", fileVersion.id},
            {"fileId", fileVersion.fileId},
            {"version", fileVersion.version},
            {"logicalNameSnapshot", fileVersion.logicalNameSnapshot},
            {"createdAt", fileVersion.createdAt}
        };
    }

    json::array toJsonArray(const std::vector<FileVersion>& versions) {
        json::array items;
        items.reserve(versions.size());

        for (const auto& version : versions) {
            items.emplace_back(toJson(version));
        }

        return items;
    }

}

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    FileVersionController::FileVersionController(FileVersionService& fileVersionService,
                                                 FileContentService& fileContentService,
                                                 AuthTokenStore& tokenStore,
                                                 ThreadPool& threadPool) noexcept
        : m_fileVersionService(fileVersionService),
          m_fileContentService(fileContentService),
          m_tokenStore(tokenStore),
          m_threadPool(threadPool) {}

    void FileVersionController::registerRoutes(Router& router) {
        router.add(http::verb::post, "/api/file-versions",
                   [this](RouteContext& ctx) {
                       handleCreateNewVersion(ctx);
                   });

        router.add(http::verb::get, "/api/file-versions/current",
                   [this](RouteContext& ctx) {
                       handleGetCurrentVersion(ctx);
                   });

        router.add(http::verb::get, "/api/file-versions",
                   [this](RouteContext& ctx) {
                       handleGetAllVersions(ctx);
                   });
    }

    void FileVersionController::handleCreateNewVersion(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto body = parseJsonObject(req.body());
        if (!body) {
            ctx.reply(makeBadRequestResponse(req, "invalid_json"));
            return;
        }

        const auto fileId = getInt64Field(*body, "fileId");
        const auto logicalNameSnapshot = getStringField(*body, "logicalNameSnapshot");

        if (!fileId || !logicalNameSnapshot || logicalNameSnapshot->empty()) {
            ctx.reply(makeBadRequestResponse(req, "fileId and logicalNameSnapshot are required"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this,
                     version,
                     keepAlive,
                     fileId = *fileId,
                     logicalNameSnapshot = *logicalNameSnapshot]() -> infrastructure::http::Response {

            auto createdStorageResult = m_fileContentService.createNew();
            if (!createdStorageResult) {
                return makeServiceErrorResponse(version, keepAlive, createdStorageResult.error());
            }

            const uint64_t physicalPath = createdStorageResult->physicalPath;

            auto createVersionResult = m_fileVersionService.createNewVersion(fileId,
                                                                            logicalNameSnapshot,
                                                                            physicalPath);

            if (!createVersionResult) {
                auto cleanupResult = m_fileContentService.remove(physicalPath);
                (void) cleanupResult;

                return makeServiceErrorResponse(version, keepAlive, createVersionResult.error());
            }

            auto currentVersionResult = m_fileVersionService.getCurrentVersion(fileId);
            if (!currentVersionResult) {
                return makeServiceErrorResponse(version, keepAlive, currentVersionResult.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::created,
                                                          serializeJson(toJson(*currentVersionResult)),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileVersionController::handleGetCurrentVersion(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
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

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId]() -> infrastructure::http::Response {
            auto result = m_fileVersionService.getCurrentVersion(fileId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson(toJson(*result)),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileVersionController::handleGetAllVersions(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
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

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId]() -> infrastructure::http::Response {
            auto result = m_fileVersionService.getAllVersions(fileId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            json::object responseBody;
            responseBody["items"] = toJsonArray(*result);

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson(responseBody),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

}