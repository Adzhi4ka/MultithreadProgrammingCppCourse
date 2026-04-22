#include "route-context.h"
#include "http-session.h"

namespace infrastructure::http {

    RouteContext::RouteContext(HttpSession& session, Request&& request)
        : m_session(session)
        , m_request(std::move(request)) {
    }

    Request& RouteContext::request() noexcept {
        return m_request;
    }

    const Request& RouteContext::request() const noexcept {
        return m_request;
    }

    void RouteContext::reply(Response response) {
        m_response = std::move(response);
    }

    bool RouteContext::hasResponse() const noexcept {
        return m_response.has_value();
    }

    Response RouteContext::takeResponse() {
        Response response = std::move(*m_response);
        m_response.reset();
        return response;
    }

    beast::tcp_stream RouteContext::releaseStream() {
        return m_session.releaseStream();
    }

    HttpSession& RouteContext::session() noexcept {
        return m_session;
    }

}