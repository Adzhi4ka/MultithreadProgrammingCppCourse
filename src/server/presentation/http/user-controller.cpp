#include "user-controller.h"

#include <boost/json/object.hpp>

#include "async-dispatch.h"
#include "auth-helpers.h"
#include "json-helpers.h"
#include "response-helpers.h"

namespace {

namespace json = boost::json;
using User = domain::models::User;

json::object toJson(const User& user) { return json::object{{"userId", user.id}, {"login", user.login}}; }

}  // namespace

namespace presentation::http {

namespace beast = boost::beast;
namespace http = beast::http;

UserController::UserController(UserService& userService, AuthTokenStore& tokenStore, ThreadPool& threadPool) noexcept
    : m_userService(userService), m_tokenStore(tokenStore), m_threadPool(threadPool) {}

void UserController::registerRoutes(Router& router) {
    router.add(http::verb::get, "/api/users/by-id", [this](RouteContext& ctx) { handleGetById(ctx); });

    router.add(http::verb::get, "/api/users/by-login", [this](RouteContext& ctx) { handleGetByLogin(ctx); });
}

void UserController::handleGetById(RouteContext& ctx) {
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
        auto result = m_userService.getById(userId);
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        return infrastructure::http::makeJsonResponse(http::status::ok, serializeJson(toJson(*result)), version,
                                                      keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

void UserController::handleGetByLogin(RouteContext& ctx) {
    auto& req = ctx.request();

    if (!authenticateUserId(req, m_tokenStore)) {
        ctx.reply(makeUnauthorizedResponse(req));
        return;
    }

    const auto login = getQueryParam(req.target(), "login");
    if (!login || login->empty()) {
        ctx.reply(makeBadRequestResponse(req, "login is required"));
        return;
    }

    const auto version = req.version();
    const bool keepAlive = req.keep_alive();
    const std::string loginValue{login->begin(), login->end()};

    auto task = [this, version, keepAlive, loginValue = std::move(loginValue)]() -> infrastructure::http::Response {
        auto result = m_userService.getByLogin(loginValue);
        if (!result) {
            return makeServiceErrorResponse(version, keepAlive, result.error());
        }

        return infrastructure::http::makeJsonResponse(http::status::ok, serializeJson(toJson(*result)), version,
                                                      keepAlive);
    };

    dispatchToWorker(ctx, m_threadPool, std::move(task));
}

}  // namespace presentation::http
