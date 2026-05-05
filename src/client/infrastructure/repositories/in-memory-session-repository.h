#pragma once

#include "domain/models/user-session.h"

#include <optional>

namespace client::infrastructure::repositories {

    class InMemorySessionRepository final {
    public:
        void save(domain::models::UserSession session) {
            m_session = std::move(session);
        }

        void clear() noexcept {
            m_session.reset();
        }

        bool hasSession() const noexcept {
            return m_session.has_value() && m_session->isAuthenticated();
        }

        const std::optional<domain::models::UserSession>& current() const noexcept {
            return m_session;
        }

    private:
        std::optional<domain::models::UserSession> m_session;
    };

}
