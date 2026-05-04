#include "auth-api.h"

#include "json-utils.h"

#include <QJsonObject>

namespace client::infrastructure::api {

    AuthApi::AuthApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void AuthApi::login(const QString& login, const QString& password, SessionCallback callback) {
        authenticate("/api/auth/login", login, password, std::move(callback));
    }

    void AuthApi::registerUser(const QString& login, const QString& password, SessionCallback callback) {
        authenticate("/api/auth/register", login, password, std::move(callback));
    }

    void AuthApi::authenticate(const QString& path,
                               const QString& login,
                               const QString& password,
                               SessionCallback callback) {
        QJsonObject body{
            {"login", login},
            {"password", password},
        };

        m_apiClient.postJson(path, body, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "authentication failed")));
                return;
            }

            QString parseError;
            const auto object = parseJsonObject(response.body, &parseError);
            if (!object) {
                callback(apiFailure(ApiError{response.httpStatus, "invalid_json", parseError}));
                return;
            }

            auto session = parseUserSession(*object);
            if (!session.isAuthenticated()) {
                callback(apiFailure(ApiError{response.httpStatus, "invalid_session", "invalid auth response"}));
                return;
            }

            callback(apiSuccess(std::move(session)));
        });
    }

}
