#include "presentation/http/test-controller.h"

#include "infrastructure/http/active-session-registry.h"
#include "infrastructure/http/listener.h"
#include "infrastructure/http/router.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

#include <csignal>
#include <iostream>
#include <string_view>

namespace {

    std::string escapeJson(std::string_view input) {
        std::string result;
        result.reserve(input.size() + 8);

        for (char ch : input) {
            switch (ch) {
                case '\\':
                    result += "\\\\";
                    break;
                case '"':
                    result += "\\\"";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    result += ch;
                    break;
            }
        }

        return result;
    }

}

int main() {
    try {
        namespace net = boost::asio;
        using tcp = net::ip::tcp;

        net::io_context ioc{1};

        infrastructure::http::Router router;
        infrastructure::http::ActiveSessionRegistry registry;

        presentation::http::DebugEventBus eventBus;
        eventBus.start();

        eventBus.subscribe<presentation::http::DebugNotificationEvent>(
            [&registry](const auto& event) {
                registry.publishToAll(
                    "debug_message",
                    "{\"message\":\"" + escapeJson(event.message) + "\"}"
                );
            }
        );

        presentation::http::TestController controller{registry};
        controller.registerRoutes(router, eventBus);

        auto listener = std::make_shared<infrastructure::http::Listener>(
            ioc,
            tcp::endpoint{tcp::v4(), 8080},
            router
        );

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code&, int) {
            eventBus.stop();
            ioc.stop();
        });

        listener->run();

        std::cout << "Server started on http://127.0.0.1:8080\n";
        ioc.run();

        eventBus.stop();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}