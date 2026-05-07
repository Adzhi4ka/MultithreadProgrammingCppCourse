#pragma once

#include <boost/asio/ip/tcp.hpp>

#include "http-session.h"
#include "router.h"

namespace infrastructure::http {

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

class Listener : public std::enable_shared_from_this<Listener> {

        net::io_context& m_ioc;
        tcp::acceptor m_acceptor;
        Router& m_router;

    public:

        Listener(net::io_context& ioc, tcp::endpoint endpoint, Router& router);

        void run();

    private:

        void doAccept();
        void onAccept(beast::error_code ec, tcp::socket socket);
};

}  // namespace infrastructure::http