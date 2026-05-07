#include "user-api.h"

#include "json-utils.h"

#include <QUrlQuery>

namespace client::infrastructure::api {

    UserApi::UserApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void UserApi::getById(qint64 userId, std::function<void(ApiResult<UserProfile>)> callback) {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("userId"), QString::number(userId));

        m_apiClient.get(QStringLiteral("/api/users/by-id"), query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseUserProfileResponse(response));
        });
    }

    void UserApi::getByLogin(const QString& login, std::function<void(ApiResult<UserProfile>)> callback) {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("login"), login);

        m_apiClient.get(QStringLiteral("/api/users/by-login"), query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseUserProfileResponse(response));
        });
    }

    ApiResult<UserApi::UserProfile> UserApi::parseUserProfileResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, QStringLiteral("failed to load user")));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, QStringLiteral("invalid_json"), parseError});
        }

        return apiSuccess(parseUserProfile(*object));
    }

}
