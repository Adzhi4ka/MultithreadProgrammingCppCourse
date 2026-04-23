#pragma once

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
            using AuthTokenStore = infrastructure::security::AuthTokenStore;
            using ThreadPool = infrastructure::execution::ThreadPool;

            FileService& m_fileService;
            FileContentService& m_fileContentService;
            AuthTokenStore& m_tokenStore;
            ThreadPool& m_threadPool;

        public:

            FileController(FileService& fileService,
                           FileContentService& fileContentService,
                           AuthTokenStore& tokenStore,
                           ThreadPool& threadPool) noexcept;

            void registerRoutes(Router& router);

        private:

            void handleCreate(RouteContext& ctx);
            void handleGetAll(RouteContext& ctx);
            void handleGetById(RouteContext& ctx);
            void handleGetByLogicalName(RouteContext& ctx);
            void handleRename(RouteContext& ctx);
            void handleRemove(RouteContext& ctx);

    };

}