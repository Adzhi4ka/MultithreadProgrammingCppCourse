#include "domain/services/user-service.h"

#include "infrastructure/database/sqlite/database-factory.h"
#include "infrastructure/database/sqlite/sqlite-database.h"
#include "infrastructure/repositories/user-repository.h"
#include "infrastructure/security/auth-token-store.h"
#include "infrastructure/security/password-hasher.h"

#include "infrastructure/http/active-session-registry.h"
#include "infrastructure/http/listener.h"
#include "infrastructure/http/router.h"

#include "presentation/http/auth-controller.h"
#include "presentation/http/notification-controller.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

#include <csignal>
#include <iostream>

int main() {
    try {
        namespace net = boost::asio;
        using tcp = net::ip::tcp;

        net::io_context ioc{1};

        infrastructure::security::initialize();

        infrastructure::db::sqlite::DatabaseFactory databaseFactory{"server.db", 2};
        infrastructure::db::sqlite::SqliteDatabase database{databaseFactory};

        infrastructure::repositories::UserRepository userRepository;
        domain::services::UserService userService{database, userRepository};

        infrastructure::security::AuthTokenStore tokenStore;
        infrastructure::http::ActiveSessionRegistry sessionRegistry;

        infrastructure::http::Router router;

        presentation::http::AuthController authController{userService, tokenStore};
        presentation::http::NotificationController notificationController{sessionRegistry, tokenStore};

        authController.registerRoutes(router);
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
        ioc.run();

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}