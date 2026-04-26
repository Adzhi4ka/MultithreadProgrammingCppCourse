#pragma once

#include "router.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <memory>
#include <optional>

namespace infrastructure::http {

    namespace net = boost::asio;
    namespace beast = boost::beast;
    using tcp = net::ip::tcp;

    class HttpSession : public std::enable_shared_from_this<HttpSession> {

            beast::tcp_stream m_stream;
            beast::flat_buffer m_buffer;
            Router& m_router;

            Request m_request;
            std::optional<Response> m_response;

            bool m_detached {false};

        public:

            HttpSession(tcp::socket&& socket, Router& router);

            void run();

            net::any_io_executor executor() noexcept;

            void sendResponse(Response response);

            beast::tcp_stream releaseStream();

        private:

            void doRead();
            void onRead(beast::error_code ec, std::size_t bytesTransferred);

            void writeResponse(Response response);
            void onWrite(beast::error_code ec, std::size_t bytesTransferred, bool keepAlive);

            void doClose();

    };

}