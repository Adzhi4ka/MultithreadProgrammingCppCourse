#pragma once

#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "domain/services/file-acl-service.h"
#include "domain/services/file-content-service.h"
#include "domain/services/file-lock-service.h"
#include "domain/services/file-service.h"
#include "domain/services/file-version-service.h"
#include "domain/services/group-service.h"
#include "domain/services/user-service.h"
#include "infrastructure/database/sqlite/database-factory.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/file-storage/file-storage.h"
#include "infrastructure/http/active-session-registry.h"
#include "infrastructure/http/listener.h"
#include "infrastructure/http/router.h"
#include "infrastructure/repositories/file-acl-repository.h"
#include "infrastructure/repositories/file-lock-repository.h"
#include "infrastructure/repositories/file-repository.h"
#include "infrastructure/repositories/file-version-repository.h"
#include "infrastructure/repositories/group-repository.h"
#include "infrastructure/repositories/user-group-repository.h"
#include "infrastructure/repositories/user-repository.h"
#include "infrastructure/security/auth-token-store.h"
#include "infrastructure/security/password-hasher.h"
#include "presentation/http/auth-controller.h"
#include "presentation/http/file-acl-controller.h"
#include "presentation/http/file-controller.h"
#include "presentation/http/file-lock-controller.h"
#include "presentation/http/file-version-controller.h"
#include "presentation/http/group-controller.h"
#include "presentation/http/notification-controller.h"

namespace tests {

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

struct AuthSession {
        int64_t userId{};
        std::string login;
        std::string token;
};

class ControllerIntegrationTest : public ::testing::Test {

    protected:

        static constexpr unsigned short kPort = 18080;

        std::string m_host{"127.0.0.1"};
        std::string m_dbPath{"controller_integration_test.db"};
        std::string m_storageDir{"controller_integration_storage"};

        std::unique_ptr<net::io_context> m_ioc;
        std::vector<std::jthread> m_ioThreads;
        std::unique_ptr<infrastructure::execution::ThreadPool> m_appThreadPool;

        std::unique_ptr<infrastructure::db::sqlite::DatabaseFactory> m_factory;
        std::unique_ptr<infrastructure::db::sqlite::SqliteDatabase> m_database;

        std::unique_ptr<infrastructure::repositories::UserRepository> m_userRepo;
        std::unique_ptr<infrastructure::repositories::GroupRepository> m_groupRepo;
        std::unique_ptr<infrastructure::repositories::UserGroupRepository> m_userGroupRepo;
        std::unique_ptr<infrastructure::repositories::FileRepository> m_fileRepo;
        std::unique_ptr<infrastructure::repositories::FileVersionRepository> m_fileVersionRepo;
        std::unique_ptr<infrastructure::repositories::FileAclRepository> m_fileAclRepo;
        std::unique_ptr<infrastructure::repositories::FileLockRepository> m_fileLockRepo;

        std::unique_ptr<domain::services::UserService> m_userService;
        std::unique_ptr<domain::services::GroupService> m_groupService;
        std::unique_ptr<domain::services::FileAclService> m_fileAclService;
        std::unique_ptr<domain::services::FileService> m_fileService;
        std::unique_ptr<domain::services::FileVersionService> m_fileVersionService;
        std::unique_ptr<domain::services::FileLockService> m_fileLockService;
        std::unique_ptr<domain::services::FileContentService> m_fileContentService;

        std::unique_ptr<infrastructure::security::AuthTokenStore> m_tokenStore;
        std::unique_ptr<infrastructure::http::ActiveSessionRegistry> m_sessionRegistry;
        std::unique_ptr<infrastructure::http::Router> m_router;

        std::unique_ptr<presentation::http::AuthController> m_authController;
        std::unique_ptr<presentation::http::GroupController> m_groupController;
        std::unique_ptr<presentation::http::FileAclController> m_fileAclController;
        std::unique_ptr<presentation::http::FileController> m_fileController;
        std::unique_ptr<presentation::http::FileVersionController> m_fileVersionController;
        std::unique_ptr<presentation::http::FileLockController> m_fileLockController;
        std::unique_ptr<presentation::http::NotificationController> m_notificationController;

        std::shared_ptr<infrastructure::http::Listener> m_listener;

    protected:

        void SetUp() override;
        void TearDown() override;

        http::response<http::string_body> request(http::verb method, std::string target, std::string body = {},
                                                  std::string bearerToken = {});

        json::value parseJson(const http::response<http::string_body>& response) const;

        AuthSession registerUser(const std::string& login, const std::string& password);
        AuthSession loginUser(const std::string& login, const std::string& password);

        int64_t createGroup(const std::string& token, const std::string& name);
        void addUserToGroup(const std::string& token, int64_t userId, int64_t groupId);

        int64_t createFile(const std::string& token, const std::string& logicalName, uint32_t maxVersionCount = 10);

        void setGroupAcl(const std::string& token, int64_t fileId, int64_t groupId, std::string_view aclLevel);

        int64_t createVersion(const std::string& token, int64_t fileId, const std::string& logicalNameSnapshot);

        std::string makeLargePayload(std::size_t size, std::uint32_t salt);
        void writeCurrentVersionContent(int64_t fileId, std::string_view bytes);
};

}  // namespace tests