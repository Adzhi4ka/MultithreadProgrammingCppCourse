#pragma once

#include <QString>

namespace client::domain::models {

struct UserProfile {
        qint64 userId = 0;
        QString login;

        bool isValid() const noexcept { return userId > 0 && !login.isEmpty(); }
};

}  // namespace client::domain::models
