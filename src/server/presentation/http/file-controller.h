#pragma once

#include "domain/notifications/notification-publisher.h"
#include "domain/services/file-acl-service.h"
#include "domain/services/file-content-service.h"
#include "domain/services/file-service.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

class FileController {

        using Router = infrastructure::http::Router;
        using RouteContext = infrastructure::http::RouteContext;
        using FileService = domain::services::FileService;
        using FileContentService = domain::services::FileContentService;
        using FileAclService = domain::services::FileAclService;
        using AuthTokenStore = infrastructure::security::AuthTokenStore;
        using ThreadPool = infrastructure::execution::ThreadPool;
        using NotificationPublisher = domain::notifications::NotificationPublisher;

        FileService& m_fileService;
        FileContentService& m_fileContentService;
        FileAclService& m_fileAclService;
        AuthTokenStore& m_tokenStore;
        ThreadPool& m_threadPool;
        NotificationPublisher* m_notificationPublisher{nullptr};

    public:

        FileController(FileService& fileService, FileContentService& fileContentService, FileAclService& fileAclService,
                       AuthTokenStore& tokenStore, ThreadPool& threadPool) noexcept;

        FileController(FileService& fileService, FileContentService& fileContentService, FileAclService& fileAclService,
                       AuthTokenStore& tokenStore, ThreadPool& threadPool,
                       NotificationPublisher& notificationPublisher) noexcept;

        void registerRoutes(Router& router);

    private:

        void handleCreate(RouteContext& ctx);
        void handleGetAll(RouteContext& ctx);
        void handleGetById(RouteContext& ctx);
        void handleGetByLogicalName(RouteContext& ctx);
        void handleRename(RouteContext& ctx);
        void handleRemove(RouteContext& ctx);
};

}  // namespace presentation::http