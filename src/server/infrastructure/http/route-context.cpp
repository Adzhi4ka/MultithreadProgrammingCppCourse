#include "route-context.h"
#include "http-session.h"

namespace infrastructure::http {

    AsyncResponseHandle::AsyncResponseHandle(std::shared_ptr<HttpSession> session)
        : m_state(std::make_shared<State>()) {
        m_state->session = std::move(session);
    }

    net::any_io_executor AsyncResponseHandle::executor() const noexcept {
        if (!m_state || !m_state->session) {
            return {};
        }

        return m_state->session->executor();
    }

    void AsyncResponseHandle::send(Response response) const {
        if (!m_state || !m_state->session) {
            return;
        }

        if (m_state->completed.exchange(true)) {
            return;
        }

        m_state->session->sendResponse(std::move(response));
    }

    AsyncResponseHandle::operator bool() const noexcept {
        return m_state && m_state->session;
    }

    RouteContext::RouteContext(std::shared_ptr<HttpSession> session, Request&& request)
        : m_session(std::move(session))
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

    AsyncResponseHandle RouteContext::deferResponse() {
        m_responseDeferred = true;
        return AsyncResponseHandle{m_session};
    }

    bool RouteContext::isResponseDeferred() const noexcept {
        return m_responseDeferred;
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
        return m_session->releaseStream();
    }

    HttpSession& RouteContext::session() noexcept {
        return *m_session;
    }

}