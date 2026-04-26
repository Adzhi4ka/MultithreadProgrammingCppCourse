#include "listener.h"

#include <boost/asio/socket_base.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/error.hpp>

namespace infrastructure::http {

    Listener::Listener(net::io_context& ioc,
                       tcp::endpoint endpoint,
                       Router& router)
        : m_ioc(ioc),
          m_acceptor(net::make_strand(ioc)),
          m_router(router) {

        beast::error_code ec;

        m_acceptor.open(endpoint.protocol(), ec);
        if (ec) {
            throw beast::system_error(ec);
        }

        m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            throw beast::system_error(ec);
        }

        m_acceptor.bind(endpoint, ec);
        if (ec) {
            throw beast::system_error(ec);
        }

        m_acceptor.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            throw beast::system_error(ec);
        }
    }

    void Listener::run() {
        doAccept();
    }

    void Listener::doAccept() {
        m_acceptor.async_accept(net::make_strand(m_ioc),
                                beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
    }

    void Listener::onAccept(beast::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<HttpSession>(std::move(socket), m_router)->run();
        }

        doAccept();
    }

}