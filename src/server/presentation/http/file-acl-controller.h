#pragma once

#include "domain/services/file-acl-service.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

    class FileAclController {

            using Router = infrastructure::http::Router;
            using RouteContext = infrastructure::http::RouteContext;
            using FileAclService = domain::services::FileAclService;
            using AuthTokenStore = infrastructure::security::AuthTokenStore;
            using ThreadPool = infrastructure::execution::ThreadPool;

            FileAclService& m_fileAclService;
            infrastructure::security::AuthTokenStore& m_tokenStore;
            infrastructure::execution::ThreadPool& m_threadPool;

        public:

            FileAclController(FileAclService& fileAclService,
                              AuthTokenStore& tokenStore,
                              ThreadPool& threadPool) noexcept;

            void registerRoutes(Router& router);

        private:

            void handleSetGroupAcl(RouteContext& ctx);
            void handleRemoveGroupAcl(RouteContext& ctx);
            void handleGetGroupAcl(RouteContext& ctx);
            void handleGetUserAcl(RouteContext& ctx);
            void handleGetFileAcls(RouteContext& ctx);
            void handleGetGroupAcls(RouteContext& ctx);

    };

}