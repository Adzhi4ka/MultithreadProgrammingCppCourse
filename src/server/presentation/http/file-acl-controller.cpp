#include "file-acl-controller.h"

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"

#include <boost/json/object.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

    using namespace domain::models;
    namespace json = boost::json;

    std::string_view toString(AclLevel aclLevel) {
        switch (aclLevel) {
            case AclLevel::NO_PROPERTY:
                return "no_property";
            case AclLevel::READ_ONLY:
                return "read_only";
            case AclLevel::READ_WRITE:
                return "read_write";
        }

        return "unknown";
    }

    std::optional<AclLevel> parseAclLevelField(const json::object& object,
                                                std::string_view fieldName) {
        const auto it = object.find(fieldName);
        if (it == object.end()) {
            return std::nullopt;
        }

        const auto& value = it->value();

        if (value.is_string()) {
            const auto& s = value.as_string();
            std::string_view text{s.data(), s.size()};

            if (text == "read_only" || text == "READ_ONLY") {
                return AclLevel::READ_ONLY;
            }

            if (text == "read_write" || text == "READ_WRITE") {
                return AclLevel::READ_WRITE;
            }

            if (text == "no_property" || text == "NO_PROPERTY") {
                return AclLevel::NO_PROPERTY;
            }

            return std::nullopt;
        }

        if (value.is_int64()) {
            switch (value.as_int64()) {
                case 0:
                    return AclLevel::NO_PROPERTY;
                case 1:
                    return AclLevel::READ_ONLY;
                case 2:
                    return AclLevel::READ_WRITE;
                default:
                    return std::nullopt;
            }
        }

        if (value.is_uint64()) {
            switch (value.as_uint64()) {
                case 0:
                    return AclLevel::NO_PROPERTY;
                case 1:
                    return AclLevel::READ_ONLY;
                case 2:
                    return AclLevel::READ_WRITE;
                default:
                    return std::nullopt;
            }
        }

        return std::nullopt;
    }

    json::object toJson(const FileAcl& fileAcl) {
        return json::object{
            {"fileId", fileAcl.fileId},
            {"groupId", fileAcl.groupId},
            {"aclLevel", std::string(toString(fileAcl.aclLevel))}
        };
    }

    json::array toJsonArray(const std::vector<FileAcl>& fileAcls) {
        json::array items;
        items.reserve(fileAcls.size());

        for (const auto& fileAcl : fileAcls) {
            items.emplace_back(toJson(fileAcl));
        }

        return items;
    }

}

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;

    FileAclController::FileAclController(FileAclService& fileAclService,
                                         AuthTokenStore& tokenStore,
                                         ThreadPool& threadPool) noexcept
        : m_fileAclService(fileAclService),
          m_tokenStore(tokenStore),
          m_threadPool(threadPool) {}

    void FileAclController::registerRoutes(infrastructure::http::Router& router) {
        router.add(http::verb::put, "/api/file-acl/groups",
                   [this](RouteContext& ctx) {
                       handleSetGroupAcl(ctx);
                   });

        router.add(http::verb::delete_, "/api/file-acl/groups",
                   [this](RouteContext& ctx) {
                       handleRemoveGroupAcl(ctx);
                   });

        router.add(http::verb::get, "/api/file-acl/groups",
                   [this](RouteContext& ctx) {
                       handleGetGroupAcl(ctx);
                   });

        router.add(http::verb::get, "/api/file-acl/users",
                   [this](RouteContext& ctx) {
                       handleGetUserAcl(ctx);
                   });

        router.add(http::verb::get, "/api/file-acl/by-file",
                   [this](RouteContext& ctx) {
                       handleGetFileAcls(ctx);
                   });

        router.add(http::verb::get, "/api/file-acl/by-group",
                   [this](RouteContext& ctx) {
                       handleGetGroupAcls(ctx);
                   });
    }

    void FileAclController::handleSetGroupAcl(RouteContext& ctx) {
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
        const auto groupId = getInt64Field(*body, "groupId");
        const auto aclLevel = parseAclLevelField(*body, "aclLevel");

        if (!fileId || !groupId || !aclLevel) {
            ctx.reply(makeBadRequestResponse(req, "fileId, groupId and aclLevel are required"));
            return;
        }

        if (*aclLevel == AclLevel::NO_PROPERTY) {
            ctx.reply(makeBadRequestResponse(req, "aclLevel must be read_only or read_write"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();
        auto task = [this, version, keepAlive, fileId = *fileId, groupId = *groupId, aclLevel = *aclLevel]() -> infrastructure::http::Response {
            auto result = m_fileAclService.setGroupAclLevel(fileId, groupId, aclLevel);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson({{"fileId", fileId},
                                                                         {"groupId", groupId},
                                                                         {"aclLevel", std::string(toString(aclLevel))}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileAclController::handleRemoveGroupAcl(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto fileIdText = getQueryParam(req.target(), "fileId");
        const auto groupIdText = getQueryParam(req.target(), "groupId");

        if (!fileIdText || !groupIdText) {
            ctx.reply(makeBadRequestResponse(req, "fileId and groupId are required"));
            return;
        }

        const auto fileId = parseInt64(*fileIdText);
        const auto groupId = parseInt64(*groupIdText);

        if (!fileId || !groupId) {
            ctx.reply(makeBadRequestResponse(req, "fileId and groupId must be int64"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId, groupId = *groupId]() -> infrastructure::http::Response {
            auto result = m_fileAclService.removeGroupAclLevel(fileId, groupId);
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

    void FileAclController::handleGetGroupAcl(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto fileIdText = getQueryParam(req.target(), "fileId");
        const auto groupIdText = getQueryParam(req.target(), "groupId");

        if (!fileIdText || !groupIdText) {
            ctx.reply(makeBadRequestResponse(req, "fileId and groupId are required"));
            return;
        }

        const auto fileId = parseInt64(*fileIdText);
        const auto groupId = parseInt64(*groupIdText);

        if (!fileId || !groupId) {
            ctx.reply(makeBadRequestResponse(req, "fileId and groupId must be int64"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId, groupId = *groupId]() -> infrastructure::http::Response {
            auto result = m_fileAclService.getGroupAclLevel(groupId, fileId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson({{"fileId", fileId},
                                                                         {"groupId", groupId},
                                                                         {"aclLevel", std::string(toString(*result))}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileAclController::handleGetUserAcl(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto fileIdText = getQueryParam(req.target(), "fileId");
        const auto userIdText = getQueryParam(req.target(), "userId");

        if (!fileIdText || !userIdText) {
            ctx.reply(makeBadRequestResponse(req, "fileId and userId are required"));
            return;
        }

        const auto fileId = parseInt64(*fileIdText);
        const auto userId = parseInt64(*userIdText);

        if (!fileId || !userId) {
            ctx.reply(makeBadRequestResponse(req, "fileId and userId must be int64"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, fileId = *fileId, userId = *userId]() -> infrastructure::http::Response {
            auto result = m_fileAclService.getUserAclLevel(userId, fileId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson({{"fileId", fileId},
                                                                         {"userId", userId},
                                                                         {"aclLevel", std::string(toString(*result))}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileAclController::handleGetFileAcls(RouteContext& ctx) {
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
            auto result = m_fileAclService.getFileAcls(fileId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson(json::object{{"items", toJsonArray(*result)}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void FileAclController::handleGetGroupAcls(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto groupIdText = getQueryParam(req.target(), "groupId");
        if (!groupIdText) {
            ctx.reply(makeBadRequestResponse(req, "groupId is required"));
            return;
        }

        const auto groupId = parseInt64(*groupIdText);
        if (!groupId) {
            ctx.reply(makeBadRequestResponse(req, "groupId must be int64"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, groupId = *groupId]() -> infrastructure::http::Response {
            auto result = m_fileAclService.getGroupAcls(groupId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson(json::object{{"items", toJsonArray(*result)}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

}