#include "group-controller.h"

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"
#include <boost/json/object.hpp>

namespace {

    using Group = domain::models::Group;
    namespace json = boost::json;

    json::object toJson(const Group& group) {
        return json::object{{"id", group.id},
                            {"name", group.name}};
    }

    json::array toJsonArray(const std::vector<Group>& groups) {
        json::array result;
        result.reserve(groups.size());

        for (const auto& group : groups) {
            result.emplace_back(toJson(group));
        }

        return result;
    }

    json::array toJsonArray(const std::vector<int64_t>& values, std::string_view fieldName) {
        json::array result;
        result.reserve(values.size());

        for (const auto value : values) {
            result.emplace_back(json::object{{fieldName, value}});
        }

        return result;
    }

}

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;
    using Group = domain::models::Group;

    GroupController::GroupController(GroupService& groupService,
                                     AuthTokenStore& tokenStore,
                                     ThreadPool& threadPool) noexcept
        : m_groupService(groupService),
          m_tokenStore(tokenStore),
          m_threadPool(threadPool) {}

    void GroupController::registerRoutes(Router& router) {
        router.add(http::verb::post, "/api/groups",
                   [this](RouteContext& ctx) { handleCreateGroup(ctx); });

        router.add(http::verb::delete_, "/api/groups",
                   [this](RouteContext& ctx) { handleDeleteGroup(ctx); });

        router.add(http::verb::get, "/api/groups",
                   [this](RouteContext& ctx) { handleGetAllGroups(ctx); });

        router.add(http::verb::get, "/api/groups/by-id",
                   [this](RouteContext& ctx) { handleGetGroupById(ctx); });

        router.add(http::verb::post, "/api/groups/members",
                   [this](RouteContext& ctx) { handleAddUserToGroup(ctx); });

        router.add(http::verb::delete_, "/api/groups/members",
                   [this](RouteContext& ctx) { handleRemoveUserFromGroup(ctx); });

        router.add(http::verb::get, "/api/groups/by-user",
                   [this](RouteContext& ctx) { handleGetUserGroups(ctx); });

        router.add(http::verb::get, "/api/groups/users",
                   [this](RouteContext& ctx) { handleGetGroupUsers(ctx); });
    }

    void GroupController::handleCreateGroup(RouteContext& ctx) {
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

        const auto groupName = getStringField(*body, "name");
        if (!groupName) {
            ctx.reply(makeBadRequestResponse(req, "name is required"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();
        const auto groupNameValue = std::move(*groupName);

        auto task = [this, version, keepAlive, groupNameValue]() -> infrastructure::http::Response {
            auto result = m_groupService.createGroup(groupNameValue);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::created,
                                                          serializeJson({{"id", *result}, {"name", groupNameValue}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void GroupController::handleDeleteGroup(RouteContext& ctx) {
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
            auto result = m_groupService.deleteGroup(groupId);
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

    void GroupController::handleGetAllGroups(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive]() -> infrastructure::http::Response {
            auto result = m_groupService.getAllGroups();
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

    void GroupController::handleGetGroupById(RouteContext& ctx) {
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
            auto result = m_groupService.getGroupById(groupId);
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

    void GroupController::handleAddUserToGroup(RouteContext& ctx) {
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

        const auto userId = getInt64Field(*body, "userId");
        const auto groupId = getInt64Field(*body, "groupId");
        if (!userId || !groupId) {
            ctx.reply(makeBadRequestResponse(req, "userId and groupId are required"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, userId = *userId, groupId = *groupId]() -> infrastructure::http::Response {
            auto result = m_groupService.addUserToGroup(userId, groupId);
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

    void GroupController::handleRemoveUserFromGroup(RouteContext& ctx) {
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

        const auto userId = getInt64Field(*body, "userId");
        const auto groupId = getInt64Field(*body, "groupId");
        if (!userId || !groupId) {
            ctx.reply(makeBadRequestResponse(req, "userId and groupId are required"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, userId = *userId, groupId = *groupId]() -> infrastructure::http::Response {
            auto result = m_groupService.removeUserFromGroup(userId, groupId);
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

    void GroupController::handleGetUserGroups(RouteContext& ctx) {
        auto& req = ctx.request();

        if (!authenticateUserId(req, m_tokenStore)) {
            ctx.reply(makeUnauthorizedResponse(req));
            return;
        }

        const auto userIdText = getQueryParam(req.target(), "userId");
        if (!userIdText) {
            ctx.reply(makeBadRequestResponse(req, "userId is required"));
            return;
        }

        const auto userId = parseInt64(*userIdText);
        if (!userId) {
            ctx.reply(makeBadRequestResponse(req, "userId must be int64"));
            return;
        }

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();

        auto task = [this, version, keepAlive, userId = *userId]() -> infrastructure::http::Response {
            auto result = m_groupService.getUserGroups(userId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson(json::object{{"items", toJsonArray(*result, "groupId")}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

    void GroupController::handleGetGroupUsers(RouteContext& ctx) {
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
            auto result = m_groupService.getGroupUsers(groupId);
            if (!result) {
                return makeServiceErrorResponse(version, keepAlive, result.error());
            }

            return infrastructure::http::makeJsonResponse(http::status::ok,
                                                          serializeJson(json::object{{"items", toJsonArray(*result, "userId")}}),
                                                          version,
                                                          keepAlive);
        };

        dispatchToWorker(ctx, m_threadPool, std::move(task));
    }

}