#pragma once

#include <optional>

#include "domain/models/user-session.h"

namespace client::infrastructure::repositories {

class SessionRepository {

        using UserSession = domain::models::UserSession;

        std::optional<UserSession> m_session;

    public:

        void save(UserSession session);
        void clear() noexcept;

        bool hasSession() const noexcept;
        const std::optional<UserSession>& current() const noexcept;
};

}  // namespace client::infrastructure::repositories
