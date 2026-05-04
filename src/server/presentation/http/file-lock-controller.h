#pragma once

#include "domain/services/file-lock-service.h"
#include "domain/notifications/notification-publisher.h"
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
            using NotificationPublisher = domain::notifications::NotificationPublisher;

            FileLockService& m_fileLockService;
            AuthTokenStore& m_tokenStore;
            ThreadPool& m_threadPool;
            NotificationPublisher* m_notificationPublisher {nullptr};

        public:

            FileLockController(FileLockService& fileLockService,
                               AuthTokenStore& tokenStore,
                               ThreadPool& threadPool) noexcept;


            FileLockController(FileLockService& fileLockService,
                               AuthTokenStore& tokenStore,
                               ThreadPool& threadPool,
                               NotificationPublisher& notificationPublisher) noexcept;

            void registerRoutes(Router& router);

        private:

            void handleAcquireLock(RouteContext& ctx);
            void handleRenewLock(RouteContext& ctx);
            void handleReleaseLock(RouteContext& ctx);
            void handleGetActiveLock(RouteContext& ctx);

    };

}