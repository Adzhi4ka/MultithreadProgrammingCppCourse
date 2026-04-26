#include "test-controller.h"

#include <charconv>
#include <string_view>

namespace {

    using infrastructure::http::Request;
    using infrastructure::http::RouteContext;
    using infrastructure::http::Response;
    namespace beast = boost::beast;
    namespace http = beast::http;

    std::string escapeJson(std::string_view input) {
        std::string result;
        result.reserve(input.size() + 8);

        for (char ch : input) {
            switch (ch) {
                case '\\':
                    result += "\\\\";
                    break;
                case '"':
                    result += "\\\"";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    result += ch;
                    break;
            }
        }

        return result;
    }

} // namespace

namespace presentation::http {

    TestController::TestController(infrastructure::http::ActiveSessionRegistry& registry) noexcept
        : m_registry(registry) {
    }

    void TestController::registerRoutes(infrastructure::http::Router& router,
                                        DebugEventBus& eventBus) {
        router.add(boost::beast::http::verb::get, "/health",
            [this](RouteContext& ctx) {
                handleHealth(ctx);
            }
        );

        router.add(boost::beast::http::verb::get, "/notifications/stream",
            [this](RouteContext& ctx) {
                handleSseStream(ctx);
            }
        );

        router.add(boost::beast::http::verb::post, "/debug/publish",
            [this, &eventBus](RouteContext& ctx) {
                handlePublish(ctx, eventBus);
            }
        );
    }

    void TestController::handleHealth(RouteContext& ctx) {
        auto& req = ctx.request();

        ctx.reply(infrastructure::http::makeTextResponse(
            boost::beast::http::status::ok,
            "OK",
            req.version(),
            req.keep_alive()
        ));
    }

    void TestController::handleSseStream(RouteContext& ctx) {
        const int64_t userId = extractUserId(ctx.request());

        auto stream = ctx.releaseStream();
        const uint64_t sessionId = m_registry.nextSessionId();

        auto sseSession = std::make_shared<infrastructure::http::SseSession>(
            std::move(stream),
            userId,
            sessionId,
            [this](int64_t closedUserId, uint64_t closedSessionId) {
                m_registry.remove(closedUserId, closedSessionId);
            }
        );

        m_registry.add(sseSession);
        sseSession->start();
        sseSession->sendEvent("connected",
                              "{\"userId\":" + std::to_string(userId) + "}");
    }

    void TestController::handlePublish(RouteContext& ctx,
                                       DebugEventBus& eventBus) {
        auto& req = ctx.request();

        eventBus.post(DebugNotificationEvent{
            .message = req.body().empty() ? std::string{"empty"} : req.body()
        });

        ctx.reply(infrastructure::http::makeJsonResponse(
            boost::beast::http::status::accepted,
            "{\"status\":\"queued\"}",
            req.version(),
            req.keep_alive()
        ));
    }

    int64_t TestController::extractUserId(const Request& request) {
        const auto header = request["X-User-Id"];
        if (header.empty()) {
            return 1;
        }

        int64_t value = 1;
        const auto* begin = header.data();
        const auto* end = header.data() + header.size();

        const auto [ptr, ec] = std::from_chars(begin, end, value);
        if (ec != std::errc{} || ptr != end) {
            return 1;
        }

        return value;
    }

}