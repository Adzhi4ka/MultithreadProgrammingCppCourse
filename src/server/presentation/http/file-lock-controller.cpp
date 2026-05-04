#include "file-lock-controller.h"

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"

#include <optional>
#include <string>

namespace {

    using namespace domain::models;
    namespace json = boost::json;

    json::object toJson(const FileLock& fileLock) {
        return json::object{
            {"fileId", fileLock.fileId},
            {"userId", fileLock.userId},
            {"leaseUntil", fileLock.leaseUntil},
            {"lockToken", fileLock.lockToken}
        };
    }

}

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;

    FileLockController::FileLockController(FileLockService& fileLockService,
                                           AuthTokenStore& tokenStore,
                                           ThreadPool& threadPool) noexcept
        : m_fileLockService(fileLockService),
          m_tokenStore(tokenStore),
          m_threadPool(threadPool) {}


    FileLockController::FileLockController(FileLockService& fileLockService,
                                           AuthTokenStore& tokenStore,
                                           ThreadPool& threadPool,
                                           NotificationPublisher& notificationPublisher) noexcept
        : FileLockController(fileLockService, tokenStore, threadPool) {
        m_notificationPublisher = &notificationPublisher;
    }

    void FileLockController::registerRoutes(Router& router) {
        router.add(http::verb::post, "/api/file-locks",
                   [this](RouteContext& ctx) {
                       handleAcquireLock(ctx);
                   });

        router.add(http::verb::put, "/api/file-locks/renew",
                   [this](RouteContext& ctx) {
                       handleRenewLock(ctx);
                   });

        router.add(http::verb::delete_, "/api/file-locks",
                   [this](RouteContext& ctx) {
                       handleReleaseLock(ctx);
                   });

        router.add(http::verb::get, "/api/file-locks/active",
                   [this](RouteContext& ctx) {
                       handleGetActiveLock(ctx);
                   });
    }

    void FileLockController::handleAcquireLock(RouteContext& ctx) {
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

        const auto fileId = getInt64Field(*body, "fileId");
        if (!fileId) {
            ctx.reply(makeBadRequestResponse(req, "fileId is required"));
            return;
        }

        const auto lockDurationSec = getInt64Field(*body, "lockDurationSec");
        if (lockDurationSec && *lockDurationSec <= 0) {
            ctx.reply(makeBadRequestResponse(req, "lockDurationSec must be positive"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive,
                     fileId = *fileId, userId = *authenticatedUserId, lockDurationSec]() -> infrastructure::http::Response {

            auto result = lockDurationSec
                ? m_fileLockService.acquireLock(fileId, userId, *lockDurationSec)
                : m_fileLockService.acquireLock(fileId, userId);

            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            if (m_notificationPublisher) {
                m_notificationPublisher->fileLocked(result->fileId,
                                                    result->userId,
                                                    result->leaseUntil,
                                                    result->lockToken);
            }

            return infrastructure::http::makeJsonResponse(http::status::created,
                                                          serializeJson(toJson(*result)),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileLockController::handleRenewLock(RouteContext& ctx) {
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
        const auto lockToken = getInt64Field(*body, "lockToken");

        if (!fileId || !lockToken) {
            ctx.reply(makeBadRequestResponse(req, "fileId and lockToken are required"));
            return;
        }

        const auto lockDurationSec = getInt64Field(*body, "lockDurationSec");
        if (lockDurationSec && *lockDurationSec <= 0) {
            ctx.reply(makeBadRequestResponse(req, "lockDurationSec must be positive"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId,
                     lockToken = *lockToken, lockDurationSec]() -> infrastructure::http::Response {

            auto result = lockDurationSec
                ? m_fileLockService.renewLock(fileId, lockToken, *lockDurationSec)
                : m_fileLockService.renewLock(fileId, lockToken);

            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }
            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          R"({"status":"ok"})",
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileLockController::handleReleaseLock(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto fileIdText = getQueryParam(req.target(), "fileId");
        const auto lockTokenText = getQueryParam(req.target(), "lockToken");

        if (!fileIdText || !lockTokenText) {
            ctx.reply(makeBadRequestResponse(req, "fileId and lockToken are required"));
            return;
        }

        const auto fileId = parseInt64(*fileIdText);
        const auto lockToken = parseInt64(*lockTokenText);

        if (!fileId || !lockToken) {
            ctx.reply(makeBadRequestResponse(req, "fileId and lockToken must be int64"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId, lockToken = *lockToken]() -> infrastructure::http::Response {

            auto result = m_fileLockService.releaseLock(fileId, lockToken);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }
            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          R"({"status":"ok"})",
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileLockController::handleGetActiveLock(RouteContext& ctx) {
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
            auto result = m_fileLockService.getActiveLock(fileId);
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

}