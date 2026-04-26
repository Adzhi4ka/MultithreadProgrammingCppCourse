#include "http-session.h"

#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/http.hpp>

namespace infrastructure::http {

    HttpSession::HttpSession(tcp::socket&& socket, Router& router)
        : m_stream(std::move(socket))
        , m_router(router) {
    }

    void HttpSession::run() {
        net::dispatch(m_stream.get_executor(),
                      beast::bind_front_handler(&HttpSession::doRead, shared_from_this()));
    }

    net::any_io_executor HttpSession::executor() noexcept {
        return m_stream.get_executor();
    }

    void HttpSession::sendResponse(Response response) {
        net::post(m_stream.get_executor(),
                  [self = shared_from_this(), response = std::move(response)]() mutable {
                      if (self->m_detached || self->m_response.has_value()) {
                          return;
                      }

                      self->writeResponse(std::move(response));
                  });
    }

    beast::tcp_stream HttpSession::releaseStream() {
        m_detached = true;
        return std::move(m_stream);
    }

    void HttpSession::doRead() {
        m_request = {};
        m_buffer.consume(m_buffer.size());

        http::async_read(m_stream,
                         m_buffer,
                         m_request,
                         beast::bind_front_handler(&HttpSession::onRead, shared_from_this()));
    }

    void HttpSession::onRead(beast::error_code ec, std::size_t) {
        if (ec == http::error::end_of_stream) {
            doClose();
            return;
        }

        if (ec) {
            return;
        }

        RouteContext context{shared_from_this(), std::move(m_request)};
        m_router.dispatch(context);

        if (m_detached || context.isResponseDeferred()) {
            return;
        }

        if (!context.hasResponse()) {
            context.reply(makeTextResponse(http::status::internal_server_error,
                                           "Handler did not produce response",
                                           context.request().version(),
                                           context.request().keep_alive()));
        }

        writeResponse(context.takeResponse());
    }

    void HttpSession::writeResponse(Response response) {
        m_response.emplace(std::move(response));
        const bool keepAlive = m_response->keep_alive();

        http::async_write(
            m_stream,
            *m_response,
            [self = shared_from_this(), keepAlive](beast::error_code ec, std::size_t bytesTransferred) {
                self->onWrite(ec, bytesTransferred, keepAlive);
            }
        );
    }

    void HttpSession::onWrite(beast::error_code ec, std::size_t, bool keepAlive) {
        if (ec) {
            return;
        }

        m_response.reset();

        if (!keepAlive) {
            doClose();
            return;
        }

        doRead();
    }

    void HttpSession::doClose() {
        beast::error_code ec;
        m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
    }

}