#pragma once

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>

#include <optional>

namespace infrastructure::http {

    namespace beast = boost::beast;
    namespace http = beast::http;

    class HttpSession;

    using Request = http::request<http::string_body>;
    using Response = http::response<http::string_body>;

    class RouteContext {

            HttpSession& m_session;
            Request m_request;
            std::optional<Response> m_response;

        public:

            RouteContext(HttpSession& session, Request&& request);

            Request& request() noexcept;
            const Request& request() const noexcept;

            void reply(Response response);

            bool hasResponse() const noexcept;
            Response takeResponse();

            beast::tcp_stream releaseStream();

            HttpSession& session() noexcept;

    };

}