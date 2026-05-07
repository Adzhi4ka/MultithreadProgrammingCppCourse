#pragma once

#include <QObject>
#include <QString>
#include <functional>

#include "api-client.h"
#include "api-result.h"
#include "domain/models/user-profile.h"

namespace client::infrastructure::api {

class UserApi : public QObject {

        using UserProfile = domain::models::UserProfile;

        Q_OBJECT
        ApiClient& m_apiClient;

    public:

        explicit UserApi(ApiClient& apiClient, QObject* parent = nullptr);

        void getById(qint64 userId, std::function<void(ApiResult<UserProfile>)> callback);
        void getByLogin(const QString& login, std::function<void(ApiResult<UserProfile>)> callback);

    private:

        static ApiResult<UserProfile> parseUserProfileResponse(const RawApiResponse& response);
};

}  // namespace client::infrastructure::api
