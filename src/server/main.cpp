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
#include "presentation/http/file-content-controller.h"
#include "presentation/http/file-controller.h"
#include "presentation/http/file-version-controller.h"
#include "presentation/http/file-lock-controller.h"
#include "presentation/http/group-controller.h"
#include "presentation/http/notification-controller.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

#include <algorithm>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <vector>

namespace infrastructure::file_storage {

    std::array<char, 124> FileStorage::m_pathBuf {};
    std::size_t FileStorage::m_prefixLen = 0;

}

namespace {

    void initializeFileStoragePrefix(std::string_view prefix) {
        using infrastructure::file_storage::FileStorage;

        if (prefix.size() + 16 + 1 > FileStorage::m_pathBuf.size()) {
            throw std::runtime_error("file storage prefix is too long");
        }

        std::fill(FileStorage::m_pathBuf.begin(), FileStorage::m_pathBuf.end(), '\0');
        std::copy(prefix.begin(), prefix.end(), FileStorage::m_pathBuf.begin());
        FileStorage::m_prefixLen = prefix.size();
    }

}

int main() {
    try {
        namespace net = boost::asio;
        using tcp = net::ip::tcp;

        const unsigned hwThreads = std::max(1u, std::thread::hardware_concurrency());
        const unsigned ioThreads = std::max(1u, hwThreads / 3);
        const unsigned workerThreads = std::max(1u, hwThreads - ioThreads);

        const std::string databasePath = "server.db";
        const std::string storageDir = "./storage";
        const std::string storagePrefix = storageDir + "/";

        std::filesystem::create_directories(storageDir);
        initializeFileStoragePrefix(storagePrefix);

        net::io_context ioc{(int)ioThreads};
        infrastructure::execution::ThreadPool appThreadPool{workerThreads};

        infrastructure::security::initialize();

        infrastructure::db::sqlite::DatabaseFactory databaseFactory{databasePath, workerThreads};
        infrastructure::db::sqlite::SqliteDatabase database{databaseFactory};

        infrastructure::repositories::UserRepository userRepository;
        infrastructure::repositories::GroupRepository groupRepository;
        infrastructure::repositories::UserGroupRepository userGroupRepository;
        infrastructure::repositories::FileRepository fileRepository;
        infrastructure::repositories::FileVersionRepository fileVersionRepository;
        infrastructure::repositories::FileAclRepository fileAclRepository;
        infrastructure::repositories::FileLockRepository fileLockRepository;

        domain::services::UserService userService{database, userRepository};
        domain::services::GroupService groupService{database, groupRepository, userGroupRepository};
        domain::services::FileService fileService{database, fileRepository, fileVersionRepository};
        domain::services::FileContentService fileContentService;
        domain::services::FileVersionService fileVersionService{database, fileRepository, fileVersionRepository};
        domain::services::FileAclService fileAclService{database, fileAclRepository, userGroupRepository};
        domain::services::FileLockService fileLockService{database, fileLockRepository};

        infrastructure::security::AuthTokenStore tokenStore;
        infrastructure::http::ActiveSessionRegistry sessionRegistry;

        infrastructure::http::Router router;

        presentation::http::AuthController authController{userService, tokenStore, appThreadPool};
        presentation::http::GroupController groupController{groupService, tokenStore, appThreadPool};
        presentation::http::FileAclController fileAclController{fileAclService, tokenStore, appThreadPool};
        presentation::http::FileLockController fileLockController{fileLockService, tokenStore, appThreadPool};
        presentation::http::FileController fileController{fileService, fileContentService, fileAclService, tokenStore, appThreadPool};
        presentation::http::FileVersionController fileVersionController{fileVersionService, fileContentService, fileAclService, tokenStore, appThreadPool};
        presentation::http::FileContentController fileContentController{fileVersionService, fileContentService, fileAclService, tokenStore, appThreadPool};
        presentation::http::NotificationController notificationController{sessionRegistry, tokenStore};

        authController.registerRoutes(router);
        groupController.registerRoutes(router);
        fileAclController.registerRoutes(router);
        fileLockController.registerRoutes(router);
        fileController.registerRoutes(router);
        fileVersionController.registerRoutes(router);
        fileContentController.registerRoutes(router);
        notificationController.registerRoutes(router);

        auto listener = std::make_shared<infrastructure::http::Listener>(
            ioc,
            tcp::endpoint{tcp::v4(), 8080},
            router
        );

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code&, int) {
            ioc.stop();
        });

        listener->run();

        std::cout << "Server started on http://127.0.0.1:8080\n";
        std::cout << "IO threads: " << ioThreads << '\n';
        std::cout << "Worker threads: " << workerThreads << '\n';

        std::vector<std::jthread> ioWorkers;
        ioWorkers.reserve(ioThreads > 1 ? ioThreads - 1 : 0);

        for (unsigned i = 1; i < ioThreads; ++i) {
            ioWorkers.emplace_back([&ioc] {
                ioc.run();
            });
        }

        ioc.run();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}