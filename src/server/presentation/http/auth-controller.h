#pragma once

#include "domain/services/user-service.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

    class AuthController {

            domain::services::UserService& m_userService;
            infrastructure::security::AuthTokenStore& m_tokenStore;

        public:

            AuthController(domain::services::UserService& userService,
                           infrastructure::security::AuthTokenStore& tokenStore) noexcept;

            void registerRoutes(infrastructure::http::Router& router);

        private:

            void handleRegister(infrastructure::http::RouteContext& ctx);
            void handleLogin(infrastructure::http::RouteContext& ctx);

            static infrastructure::http::Response makeServiceErrorResponse(const infrastructure::http::Request& request,
                                                                           domain::services::ServiceError error);

    };

}