#pragma once

#include "api-client.h"
#include "api-result.h"
#include "domain/models/user-session.h"

#include <QObject>
#include <QString>

#include <functional>

namespace client::infrastructure::api {

    class AuthApi : public QObject {

            using UserSession = domain::models::UserSession;
    
            Q_OBJECT
    
            ApiClient& m_apiClient;
    
        public:
            explicit AuthApi(ApiClient& apiClient, QObject* parent = nullptr);
    
            void login(const QString& login,
                       const QString& password,
                       std::function<void(ApiResult<domain::models::UserSession>)> callback);
            void registerUser(const QString& login,
                              const QString& password,
                              std::function<void(ApiResult<domain::models::UserSession>)> callback);
    
        private:
            void authenticate(const QString& path,
                              const QString& login,
                              const QString& password,
                              std::function<void(ApiResult<domain::models::UserSession>)> callback);

    };

}
