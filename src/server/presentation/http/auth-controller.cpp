#include "auth-controller.h"
#include "async-dispatch.h"
#include "json-helpers.h"

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    AuthController::AuthController(domain::services::UserService& userService,
                                   infrastructure::security::AuthTokenStore& tokenStore,
                                   infrastructure::execution::ThreadPool& threadPool) noexcept
        : m_userService(userService),
          m_tokenStore(tokenStore),
          m_threadPool(threadPool) {}

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

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();
        const auto loginValue = *login;
        const auto passwordValue = *password;

        dispatchToWorker(ctx,
                         m_threadPool,
                         [this, version, keepAlive, loginValue, passwordValue]() -> infrastructure::http::Response {
                             auto result = m_userService.addUser(loginValue, passwordValue);
                             if (!result) {
                                 return makeServiceErrorResponse(version, keepAlive, result.error());
                             }

                             const auto token = m_tokenStore.issueToken(*result);

                             json::object responseBody{{"userId", *result},
                                                       {"login", loginValue},
                                                       {"token", token}};

                             return infrastructure::http::makeJsonResponse(http::status::created,
                                                                           serializeJson(responseBody),
                                                                           version,
                                                                           keepAlive);
                         });
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

        const auto version = req.version();
        const bool keepAlive = req.keep_alive();
        const auto& loginValue = *login;
        const auto& passwordValue = *password;

        dispatchToWorker(ctx,
                         m_threadPool,
                         [this, version, keepAlive, loginValue, passwordValue]() -> infrastructure::http::Response {
                             auto result = m_userService.login(loginValue, passwordValue);
                             if (!result) {
                                 return makeServiceErrorResponse(version, keepAlive, result.error());
                             }

                             json::object responseBody{{"userId", *result},
                                                       {"login", loginValue},
                                                       {"token", m_tokenStore.issueToken(*result)}};

                             return infrastructure::http::makeJsonResponse(http::status::ok,
                                                                           serializeJson(responseBody),
                                                                           version,
                                                                           keepAlive);
                         });
    }

    infrastructure::http::Response AuthController::makeServiceErrorResponse(
        unsigned version,
        bool keepAlive,
        domain::services::ServiceError error) {

        switch (error) {
            case domain::services::ServiceError::Conflict:
                return infrastructure::http::makeJsonResponse(http::status::conflict,
                                                              R"({"error":"conflict"})",
                                                              version,
                                                              keepAlive);

            case domain::services::ServiceError::Forbidden:
                return infrastructure::http::makeJsonResponse(http::status::unauthorized,
                                                              R"({"error":"invalid_credentials"})",
                                                              version,
                                                              keepAlive);

            case domain::services::ServiceError::NotFound:
                return infrastructure::http::makeJsonResponse(http::status::not_found,
                                                              R"({"error":"not_found"})",
                                                              version,
                                                              keepAlive);

            case domain::services::ServiceError::InternalError:
            default:
                return infrastructure::http::makeJsonResponse(http::status::internal_server_error,
                                                              R"({"error":"internal_error"})",
                                                              version,
                                                              keepAlive);
        }
    }

}