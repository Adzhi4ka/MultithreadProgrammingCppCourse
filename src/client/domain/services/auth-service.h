#pragma once

#include <QUrl>
#include <functional>

#include "domain/services/remote-service-base.h"
#include "infrastructure/repositories/session-repository.h"

namespace client::domain::services {

class AuthService : public RemoteServiceBase {

        using UserSession = domain::models::UserSession;
        using RemoteApiGateway = infrastructure::api::RemoteApiGateway;

        infrastructure::repositories::SessionRepository& m_sessionRepository;

    public:

        AuthService(application::NetworkWorker& networkWorker, QObject& internalContext, QObject& uiContext,
                    infrastructure::repositories::SessionRepository& sessionRepository) noexcept;

        void authenticate(QUrl baseUrl, QString login, QString password, bool registration,
                          std::function<void(ApiResult<UserSession>)> callback);
        void setSession(UserSession session);
        void logout();
};

}  // namespace client::domain::services
