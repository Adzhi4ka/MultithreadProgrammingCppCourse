#pragma once

#include "domain/services/remote-service-base.h"
#include "infrastructure/repositories/session-repository.h"

#include <QUrl>

#include <functional>

namespace client::domain::services {

    class AuthService : public RemoteServiceBase {

            using UserSession = domain::models::UserSession;
            infrastructure::repositories::SessionRepository& m_sessionRepository;

        public:
            AuthService(application::NetworkWorker& networkWorker,
                        QObject& internalContext,
                        QObject& uiContext,
                        infrastructure::repositories::SessionRepository& sessionRepository) noexcept;

            void authenticate(QUrl baseUrl,
                            QString login,
                            QString password,
                            bool registration,
                            std::function<void(ApiResult<UserSession>)> callback);
            void setSession(UserSession session);
            void logout();

    };

}
