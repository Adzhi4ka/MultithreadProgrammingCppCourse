#include "notification-controller.h"
#include "auth-helpers.h"
#include "json-helpers.h"

#include <boost/json.hpp>

namespace presentation::http {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace json = boost::json;

    NotificationController::NotificationController(infrastructure::http::ActiveSessionRegistry& registry,
                                                   infrastructure::security::AuthTokenStore& tokenStore) noexcept
        : m_registry(registry),
          m_tokenStore(tokenStore) {}

    void NotificationController::registerRoutes(infrastructure::http::Router& router) {
        router.add(http::verb::get, "/api/notifications/stream",
            [this](infrastructure::http::RouteContext& ctx) {
                handleStream(ctx);
            }
        );
    }

    void NotificationController::handleStream(infrastructure::http::RouteContext& ctx) {
        const auto userId = authenticateUserId(ctx.request(), m_tokenStore);
        if (!userId) {
            ctx.reply(makeUnauthorizedResponse(ctx.request()));
            return;
        }

        auto stream = ctx.releaseStream();
        const auto sessionId = m_registry.nextSessionId();

        auto sseSession =
            std::make_shared<infrastructure::http::SseSession>(std::move(stream),
                                                               *userId,
                                                               sessionId,
                                                               [this](int64_t closedUserId, uint64_t closedSessionId) {
                                                                   m_registry.remove(closedUserId, closedSessionId);
                                                               });

        m_registry.add(sseSession);
        sseSession->start();

        json::object payload{
            {"userId", *userId}
        };

        sseSession->sendEvent("connected", serializeJson(payload));
    }

}