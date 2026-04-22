#include "auth-controller.h"
#include "json-helpers.h"

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    AuthController::AuthController(domain::services::UserService& userService,
                                   infrastructure::security::AuthTokenStore& tokenStore) noexcept
        : m_userService(userService),
          m_tokenStore(tokenStore) {}

    void AuthController::registerRoutes(infrastructure::http::Router& router) {
        router.add(http::verb::post, "/api/auth/register",
            [this](infrastructure::http::RouteContext& ctx) {
                handleRegister(ctx);
            }
        );

        router.add(http::verb::post, "/api/auth/login",
            [this](infrastructure::http::RouteContext& ctx) {
                handleLogin(ctx);
            }
        );
    }

    void AuthController::handleRegister(infrastructure::http::RouteContext& ctx) {
        auto& req = ctx.request();

        const auto body = parseJsonObject(req.body());
        if (!body) {
            ctx.reply(infrastructure::http::makeJsonResponse(http::status::bad_request,
                                                             R"({"error":"invalid_json"})",
                                                             req.version(),
                                                             req.keep_alive()));

            return;
        }

        const auto login = getStringField(*body, "login");
        const auto password = getStringField(*body, "password");

        if (!login || !password) {
            ctx.reply(infrastructure::http::makeJsonResponse(http::status::bad_request,
                                                             R"({"error":"login and password are required"})",
                                                             req.version(),
                                                             req.keep_alive()));

            return;
        }

        try {
            auto result = m_userService.addUser(*login, *password);
            if (!result) {
                ctx.reply(makeServiceErrorResponse(req, result.error()));
                return;
            }

            const auto token = m_tokenStore.issueToken(*result);

            json::object responseBody{{"userId", *result},
                                      {"login", *login},
                                      {"token", token}};

            ctx.reply(infrastructure::http::makeJsonResponse(http::status::created,
                                                             serializeJson(responseBody),
                                                             req.version(),
                                                             req.keep_alive()));
        } catch (const std::exception&) {
            ctx.reply(infrastructure::http::makeJsonResponse(http::status::internal_server_error,
                                                             R"({"error":"internal_error"})",
                                                             req.version(),
                                                             req.keep_alive()));
        }
    }

    void AuthController::handleLogin(infrastructure::http::RouteContext& ctx) {
        auto& req = ctx.request();

        const auto body = parseJsonObject(req.body());
        if (!body) {
            ctx.reply(infrastructure::http::makeJsonResponse(http::status::bad_request,
                                                             R"({"error":"invalid_json"})",
                                                             req.version(),
                                                             req.keep_alive()));
            return;
        }

        const auto login = getStringField(*body, "login");
        const auto password = getStringField(*body, "password");

        if (!login || !password) {
            ctx.reply(infrastructure::http::makeJsonResponse(http::status::bad_request,
                                                             R"({"error":"login and password are required"})",
                                                             req.version(),
                                                             req.keep_alive()));
            return;
        }

        auto result = m_userService.login(*login, *password);
        if (!result) {
            ctx.reply(makeServiceErrorResponse(req, result.error()));
            return;
        }

        const auto token = m_tokenStore.issueToken(*result);

        json::object responseBody{{"userId", *result},
                                  {"login", *login},
                                  {"token", token}};

        ctx.reply(infrastructure::http::makeJsonResponse(http::status::ok,
                                                         serializeJson(responseBody),
                                                         req.version(),
                                                         req.keep_alive()));
    }

    infrastructure::http::Response AuthController::makeServiceErrorResponse(
        const infrastructure::http::Request& request,
        domain::services::ServiceError error) {

        switch (error) {
            case domain::services::ServiceError::Conflict:
                return infrastructure::http::makeJsonResponse(http::status::conflict,
                                                              R"({"error":"conflict"})",
                                                              request.version(),
                                                              request.keep_alive());

            case domain::services::ServiceError::Forbidden:
                return infrastructure::http::makeJsonResponse(http::status::unauthorized,
                                                              R"({"error":"invalid_credentials"})",
                                                              request.version(),
                                                              request.keep_alive());

            case domain::services::ServiceError::NotFound:
                return infrastructure::http::makeJsonResponse(http::status::not_found,
                                                              R"({"error":"not_found"})",
                                                              request.version(),
                                                              request.keep_alive());

            case domain::services::ServiceError::InternalError:
            default:
                return infrastructure::http::makeJsonResponse(http::status::internal_server_error,
                                                              R"({"error":"internal_error"})",
                                                              request.version(),
                                                              request.keep_alive());
        }
    }

}