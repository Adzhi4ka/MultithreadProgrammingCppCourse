#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/user-session.h"

#include <QObject>
#include <QString>

#include <functional>

namespace client::infrastructure::api {

    class AuthApi final : public QObject {
        Q_OBJECT

    public:
        using SessionResult = ApiResult<domain::models::UserSession>;
        using SessionCallback = std::function<void(SessionResult)>;

        explicit AuthApi(ApiClient& apiClient, QObject* parent = nullptr);

        void login(const QString& login, const QString& password, SessionCallback callback);
        void registerUser(const QString& login, const QString& password, SessionCallback callback);

    private:
        void authenticate(const QString& path, const QString& login, const QString& password, SessionCallback callback);

    private:
        ApiClient& m_apiClient;
    };

}
