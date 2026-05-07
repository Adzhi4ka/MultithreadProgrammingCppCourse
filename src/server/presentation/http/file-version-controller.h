#pragma once

#include "domain/services/file-acl-service.h"
#include "domain/services/file-content-service.h"
#include "domain/services/file-version-service.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

class FileVersionController {

        using Router = infrastructure::http::Router;
        using RouteContext = infrastructure::http::RouteContext;
        using FileVersionService = domain::services::FileVersionService;
        using FileContentService = domain::services::FileContentService;
        using FileAclService = domain::services::FileAclService;
        using AuthTokenStore = infrastructure::security::AuthTokenStore;
        using ThreadPool = infrastructure::execution::ThreadPool;

        FileVersionService& m_fileVersionService;
        FileContentService& m_fileContentService;
        FileAclService& m_fileAclService;
        AuthTokenStore& m_tokenStore;
        ThreadPool& m_threadPool;

    public:

        FileVersionController(FileVersionService& fileVersionService, FileContentService& fileContentService,
                              FileAclService& fileAclService, AuthTokenStore& tokenStore,
                              ThreadPool& threadPool) noexcept;

        void registerRoutes(Router& router);

    private:

        void handleCreateNewVersion(RouteContext& ctx);
        void handleGetCurrentVersion(RouteContext& ctx);
        void handleGetAllVersions(RouteContext& ctx);
};

}  // namespace presentation::http