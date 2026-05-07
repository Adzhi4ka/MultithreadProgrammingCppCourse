#pragma once

#include <QString>

namespace client::domain::models {

struct UserSession {
        qint64 userId = 0;
        QString login;
        QString token;

        bool isAuthenticated() const noexcept { return userId > 0 && !token.isEmpty(); }
};

}  // namespace client::domain::models
