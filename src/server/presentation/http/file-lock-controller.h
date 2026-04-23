#pragma once

#include "domain/services/file-lock-service.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

    class FileLockController {

            using Router = infrastructure::http::Router;
            using RouteContext = infrastructure::http::RouteContext;
            using FileLockService = domain::services::FileLockService;
            using AuthTokenStore = infrastructure::security::AuthTokenStore;
            using ThreadPool = infrastructure::execution::ThreadPool;

            FileLockService& m_fileLockService;
            AuthTokenStore& m_tokenStore;
            ThreadPool& m_threadPool;

        public:

            FileLockController(FileLockService& fileLockService,
                               AuthTokenStore& tokenStore,
                               ThreadPool& threadPool) noexcept;

            void registerRoutes(Router& router);

        private:

            void handleAcquireLock(RouteContext& ctx);
            void handleRenewLock(RouteContext& ctx);
            void handleReleaseLock(RouteContext& ctx);
            void handleGetActiveLock(RouteContext& ctx);

    };

}