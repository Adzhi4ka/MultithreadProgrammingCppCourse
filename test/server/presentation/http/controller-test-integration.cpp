#include "controller-test-integration.h"

#include <boost/beast/http/string_body.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <system_error>

#include <fcntl.h>
#include <unistd.h>

namespace infrastructure::file_storage {

    std::array<char, 124> FileStorage::m_pathBuf {};
    std::size_t FileStorage::m_prefixLen = 0;

}

namespace {

    void initializeFileStoragePrefix(const std::string& prefix) {
        using infrastructure::file_storage::FileStorage;

        if (prefix.size() + 16 + 1 > FileStorage::m_pathBuf.size()) {
            throw std::runtime_error("storage prefix is too long");
        }

        std::fill(FileStorage::m_pathBuf.begin(), FileStorage::m_pathBuf.end(), '\0');
        std::copy(prefix.begin(), prefix.end(), FileStorage::m_pathBuf.begin());
        FileStorage::m_prefixLen = prefix.size();
    }

    void writeAllToFd(int fd, std::string_view data) {
        std::size_t written = 0;

        while (written < data.size()) {
            ssize_t rc = ::write(fd, data.data() + written, data.size() - written);

            if (rc == -1 && errno == EINTR) {
                continue;
            }

            if (rc <= 0) {
                throw std::system_error(errno, std::generic_category(), "write failed");
            }

            written += rc;
        }
    }

}

namespace tests {

    void ControllerIntegrationTest::SetUp() {
        using namespace std::chrono_literals;

        std::filesystem::remove(m_dbPath);
        std::filesystem::remove_all(m_storageDir);
        std::filesystem::create_directories(m_storageDir);

        infrastructure::security::initialize();
        initializeFileStoragePrefix(m_storageDir + "/");

        const unsigned hwThreads = std::max(2u, std::thread::hardware_concurrency());
        const unsigned ioThreads = 2;
        const unsigned workerThreads = std::max(2u, hwThreads / 2);

        m_ioc = std::make_unique<net::io_context>(ioThreads);
        m_appThreadPool = std::make_unique<infrastructure::execution::ThreadPool>(workerThreads);

        m_factory = std::make_unique<infrastructure::db::sqlite::DatabaseFactory>(m_dbPath, workerThreads);
        m_database = std::make_unique<infrastructure::db::sqlite::SqliteDatabase>(*m_factory);

        m_userRepo = std::make_unique<infrastructure::repositories::UserRepository>();
        m_groupRepo = std::make_unique<infrastructure::repositories::GroupRepository>();
        m_userGroupRepo = std::make_unique<infrastructure::repositories::UserGroupRepository>();
        m_fileRepo = std::make_unique<infrastructure::repositories::FileRepository>();
        m_fileVersionRepo = std::make_unique<infrastructure::repositories::FileVersionRepository>();
        m_fileAclRepo = std::make_unique<infrastructure::repositories::FileAclRepository>();
        m_fileLockRepo = std::make_unique<infrastructure::repositories::FileLockRepository>();

        m_userService = std::make_unique<domain::services::UserService>(*m_database, *m_userRepo);
        m_groupService = std::make_unique<domain::services::GroupService>(*m_database,
                                                                          *m_groupRepo,
                                                                          *m_userGroupRepo);
        m_fileAclService = std::make_unique<domain::services::FileAclService>(*m_database,
                                                                              *m_fileAclRepo,
                                                                              *m_userGroupRepo);
        m_fileService = std::make_unique<domain::services::FileService>(*m_database,
                                                                        *m_fileRepo,
                                                                        *m_fileVersionRepo);
        m_fileVersionService = std::make_unique<domain::services::FileVersionService>(*m_database,
                                                                                      *m_fileRepo,
                                                                                      *m_fileVersionRepo);
        m_fileLockService = std::make_unique<domain::services::FileLockService>(*m_database,
                                                                                *m_fileLockRepo);
        m_fileContentService = std::make_unique<domain::services::FileContentService>();

        m_tokenStore = std::make_unique<infrastructure::security::AuthTokenStore>();
        m_sessionRegistry = std::make_unique<infrastructure::http::ActiveSessionRegistry>();
        m_router = std::make_unique<infrastructure::http::Router>();

        m_authController = std::make_unique<presentation::http::AuthController>(*m_userService,
                                                                                *m_tokenStore,
                                                                                *m_appThreadPool);
        m_groupController = std::make_unique<presentation::http::GroupController>(*m_groupService,
                                                                                  *m_tokenStore,
                                                                                  *m_appThreadPool);
        m_fileAclController = std::make_unique<presentation::http::FileAclController>(*m_fileAclService,
                                                                                      *m_tokenStore,
                                                                                      *m_appThreadPool);
        m_fileController = std::make_unique<presentation::http::FileController>(*m_fileService,
                                                                                *m_fileContentService,
                                                                                *m_fileAclService,
                                                                                *m_tokenStore,
                                                                                *m_appThreadPool);
        m_fileVersionController = std::make_unique<presentation::http::FileVersionController>(*m_fileVersionService,
                                                                                              *m_fileContentService,
                                                                                              *m_fileAclService,
                                                                                              *m_tokenStore,
                                                                                              *m_appThreadPool);
        m_fileLockController = std::make_unique<presentation::http::FileLockController>(*m_fileLockService,
                                                                                        *m_tokenStore,
                                                                                        *m_appThreadPool);
        m_notificationController = std::make_unique<presentation::http::NotificationController>(*m_sessionRegistry,
                                                                                                *m_tokenStore);

        m_authController->registerRoutes(*m_router);
        m_groupController->registerRoutes(*m_router);
        m_fileAclController->registerRoutes(*m_router);
        m_fileController->registerRoutes(*m_router);
        m_fileVersionController->registerRoutes(*m_router);
        m_fileLockController->registerRoutes(*m_router);
        m_notificationController->registerRoutes(*m_router);

        m_listener = std::make_shared<infrastructure::http::Listener>(*m_ioc,
                                                                      net::ip::tcp::endpoint{net::ip::tcp::v4(), kPort},
                                                                      *m_router);
        m_listener->run();

        m_ioThreads.reserve(ioThreads);
        for (unsigned i = 0; i < ioThreads; ++i) {
            m_ioThreads.emplace_back([this] {
                m_ioc->run();
            });
        }

        std::this_thread::sleep_for(50ms);
    }

    void ControllerIntegrationTest::TearDown() {
        m_listener.reset();

        if (m_ioc) {
            m_ioc->stop();
        }

        m_ioThreads.clear();

        m_notificationController.reset();
        m_fileLockController.reset();
        m_fileVersionController.reset();
        m_fileController.reset();
        m_fileAclController.reset();
        m_groupController.reset();
        m_authController.reset();

        m_router.reset();
        m_sessionRegistry.reset();
        m_tokenStore.reset();

        m_fileContentService.reset();
        m_fileLockService.reset();
        m_fileVersionService.reset();
        m_fileService.reset();
        m_fileAclService.reset();
        m_groupService.reset();
        m_userService.reset();

        m_fileLockRepo.reset();
        m_fileAclRepo.reset();
        m_fileVersionRepo.reset();
        m_fileRepo.reset();
        m_userGroupRepo.reset();
        m_groupRepo.reset();
        m_userRepo.reset();

        m_database.reset();
        m_factory.reset();

        m_appThreadPool.reset();
        m_ioc.reset();

        std::filesystem::remove(m_dbPath);
        std::filesystem::remove_all(m_storageDir);
    }

    http::response<http::string_body> ControllerIntegrationTest::request(http::verb method,
                                                                         std::string target,
                                                                         std::string body,
                                                                         std::string bearerToken) {
        net::io_context ioc;
        net::ip::tcp::resolver resolver{ioc};
        beast::tcp_stream stream{ioc};

        const auto results = resolver.resolve(m_host, std::to_string(kPort));
        stream.connect(results);

        http::request<http::string_body> req{method, std::move(target), 11};
        req.set(http::field::host, m_host);
        req.set(http::field::user_agent, "controller-integration-test");

        if (!bearerToken.empty()) {
            req.set(http::field::authorization, "Bearer " + bearerToken);
        }

        if (!body.empty()) {
            req.set(http::field::content_type, "application/json");
            req.body() = std::move(body);
            req.prepare_payload();
        }

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> response;
        http::read(stream, buffer, response);

        beast::error_code ec;
        stream.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);

        return response;
    }

    json::value ControllerIntegrationTest::parseJson(const http::response<http::string_body>& response) const {
        return json::parse(response.body());
    }

    AuthSession ControllerIntegrationTest::registerUser(const std::string& login, const std::string& password) {
        json::object body{
            {"login", login},
            {"password", password}
        };

        auto response = request(http::verb::post,
                                "/api/auth/register",
                                json::serialize(body));

        EXPECT_EQ(response.result(), http::status::created) << response.body();

        auto value = parseJson(response).as_object();

        return AuthSession{
            .userId = value.at("userId").as_int64(),
            .login = std::string(value.at("login").as_string().c_str()),
            .token = std::string(value.at("token").as_string().c_str())
        };
    }

    AuthSession ControllerIntegrationTest::loginUser(const std::string& login, const std::string& password) {
        json::object body{
            {"login", login},
            {"password", password}
        };

        auto response = request(http::verb::post,
                                "/api/auth/login",
                                json::serialize(body));

        EXPECT_EQ(response.result(), http::status::ok) << response.body();

        auto value = parseJson(response).as_object();

        return AuthSession{
            .userId = value.at("userId").as_int64(),
            .login = std::string(value.at("login").as_string().c_str()),
            .token = std::string(value.at("token").as_string().c_str())
        };
    }

    int64_t ControllerIntegrationTest::createGroup(const std::string& token, const std::string& name) {
        json::object body{
            {"name", name}
        };

        auto response = request(http::verb::post,
                                "/api/groups",
                                json::serialize(body),
                                token);

        EXPECT_EQ(response.result(), http::status::created) << response.body();

        return parseJson(response).as_object().at("id").as_int64();
    }

    void ControllerIntegrationTest::addUserToGroup(const std::string& token, int64_t userId, int64_t groupId) {
        json::object body{
            {"userId", userId},
            {"groupId", groupId}
        };

        auto response = request(http::verb::post,
                                "/api/groups/members",
                                json::serialize(body),
                                token);

        EXPECT_EQ(response.result(), http::status::ok) << response.body();
    }

    int64_t ControllerIntegrationTest::createFile(const std::string& token,
                                                  const std::string& logicalName,
                                                  uint32_t maxVersionCount) {
        json::object body{
            {"logicalName", logicalName},
            {"maxVersionCount", maxVersionCount}
        };

        auto response = request(http::verb::post,
                                "/api/files",
                                json::serialize(body),
                                token);

        EXPECT_EQ(response.result(), http::status::created) << response.body();

        return parseJson(response).as_object().at("id").as_int64();
    }

    void ControllerIntegrationTest::setGroupAcl(const std::string& token,
                                                int64_t fileId,
                                                int64_t groupId,
                                                std::string_view aclLevel) {
        json::object body{
            {"fileId", fileId},
            {"groupId", groupId},
            {"aclLevel", std::string(aclLevel)}
        };

        auto response = request(http::verb::put,
                                "/api/file-acl/groups",
                                json::serialize(body),
                                token);

        EXPECT_EQ(response.result(), http::status::ok) << response.body();
    }

    int64_t ControllerIntegrationTest::createVersion(const std::string& token,
                                                     int64_t fileId,
                                                     const std::string& logicalNameSnapshot) {
        json::object body{
            {"fileId", fileId},
            {"logicalNameSnapshot", logicalNameSnapshot}
        };

        auto response = request(http::verb::post,
                                "/api/file-versions",
                                json::serialize(body),
                                token);

        EXPECT_EQ(response.result(), http::status::created) << response.body();

        return parseJson(response).as_object().at("id").as_int64();
    }

    std::string ControllerIntegrationTest::makeLargePayload(std::size_t size, std::uint32_t salt) {
        std::string data(size, '\0');

        for (std::size_t i = 0; i < size; ++i) {
            data[i] = 'a' + ((i + salt) % 26);
        }

        return data;
    }

    void ControllerIntegrationTest::writeCurrentVersionContent(int64_t fileId, std::string_view bytes) {
        auto currentVersionResult = m_fileVersionService->getCurrentVersion(fileId);
        ASSERT_TRUE(currentVersionResult.has_value());

        auto storage = infrastructure::file_storage::FileStorage::openReadWrite(currentVersionResult->physicalPathName);

        ASSERT_NE(::ftruncate(storage.getFd(), 0), -1);

        writeAllToFd(storage.getFd(), bytes);
    }

} // namespace tests