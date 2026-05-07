#pragma once

#include "domain/services/user-service.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

    class UserController {

            using RouteContext = infrastructure::http::RouteContext;
            using Router = infrastructure::http::Router;
            using UserService = domain::services::UserService;
            using AuthTokenStore = infrastructure::security::AuthTokenStore;
            using ThreadPool = infrastructure::execution::ThreadPool;

            UserService& m_userService;
            AuthTokenStore& m_tokenStore;
            ThreadPool& m_threadPool;

        public:
            UserController(UserService& userService,
                           AuthTokenStore& tokenStore,
                           ThreadPool& threadPool) noexcept;

            void registerRoutes(Router& router);

        private:
            void handleGetById(RouteContext& ctx);
            void handleGetByLogin(RouteContext& ctx);

    };

}
