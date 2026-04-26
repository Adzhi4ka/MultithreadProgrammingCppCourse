#pragma once

#include "infrastructure/event/event-bus.h"
#include "infrastructure/http/active-session-registry.h"
#include "infrastructure/http/router.h"
#include "infrastructure/http/sse-session.h"

#include <cstdint>
#include <string>

namespace presentation::http {

    struct DebugNotificationEvent {
        std::string message;
    };

    using DebugEventBus = domain::notifications::events::EventBus<DebugNotificationEvent>;

    class TestController {

            infrastructure::http::ActiveSessionRegistry& m_registry;

        public:

            explicit TestController(infrastructure::http::ActiveSessionRegistry& registry) noexcept;

            void registerRoutes(infrastructure::http::Router& router,
                                DebugEventBus& eventBus);

        private:

            void handleHealth(infrastructure::http::RouteContext& ctx);

            void handleSseStream(infrastructure::http::RouteContext& ctx);

            void handlePublish(infrastructure::http::RouteContext& ctx,
                               DebugEventBus& eventBus);

        private:

            static int64_t extractUserId(const infrastructure::http::Request& request);

    };

}