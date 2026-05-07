#pragma once

#include "infrastructure/http/active-session-registry.h"
#include "infrastructure/http/router.h"
#include "infrastructure/http/sse-session.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

class NotificationController {

        infrastructure::http::ActiveSessionRegistry& m_registry;
        infrastructure::security::AuthTokenStore& m_tokenStore;

    public:

        NotificationController(infrastructure::http::ActiveSessionRegistry& registry,
                               infrastructure::security::AuthTokenStore& tokenStore) noexcept;

        void registerRoutes(infrastructure::http::Router& router);

    private:

        void handleStream(infrastructure::http::RouteContext& ctx);
};

}  // namespace presentation::http