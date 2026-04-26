#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>

#include <atomic>
#include <memory>
#include <optional>

namespace infrastructure::http {

    namespace net = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;

    class HttpSession;

    using Request = http::request<http::string_body>;
    using Response = http::response<http::string_body>;

    class AsyncResponseHandle {

            struct State {
                std::shared_ptr<HttpSession> session;
                std::atomic_bool completed {false};
            };

            std::shared_ptr<State> m_state;

        public:

            AsyncResponseHandle() = default;
            explicit AsyncResponseHandle(std::shared_ptr<HttpSession> session);

            net::any_io_executor executor() const noexcept;

            void send(Response response) const;

            explicit operator bool() const noexcept;

    };

    class RouteContext {

            std::shared_ptr<HttpSession> m_session;
            Request m_request;
            std::optional<Response> m_response;
            bool m_responseDeferred {false};

        public:

            RouteContext(std::shared_ptr<HttpSession> session, Request&& request);

            Request& request() noexcept;
            const Request& request() const noexcept;

            void reply(Response response);

            AsyncResponseHandle deferResponse();
            bool isResponseDeferred() const noexcept;

            bool hasResponse() const noexcept;
            Response takeResponse();

            beast::tcp_stream releaseStream();

            HttpSession& session() noexcept;

    };

}