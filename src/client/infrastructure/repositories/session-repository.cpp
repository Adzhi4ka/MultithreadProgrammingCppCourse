#include "session-repository.h"

#include <utility>

namespace {
    using UserSession = client::domain::models::UserSession;
}

namespace client::infrastructure::repositories {

    void SessionRepository::save(UserSession session) {
        m_session = std::move(session);
    }

    void SessionRepository::clear() noexcept {
        m_session.reset();
    }

    bool SessionRepository::hasSession() const noexcept {
        return m_session.has_value() && m_session->isAuthenticated();
    }

    const std::optional<UserSession>& SessionRepository::current() const noexcept {
        return m_session;
    }

}
