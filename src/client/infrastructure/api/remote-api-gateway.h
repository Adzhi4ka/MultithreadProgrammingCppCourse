#pragma once

#include "api-client.h"
#include "auth-api.h"
#include "file-acl-api.h"
#include "file-api.h"
#include "file-content-api.h"
#include "file-lock-api.h"
#include "file-version-api.h"
#include "group-api.h"
#include "notification-stream-client.h"
#include "user-api.h"

#include <QUrl>

#include <functional>

namespace client::infrastructure::api {

    class RemoteApiGateway {

            ApiClient m_apiClient;
            AuthApi m_authApi;
            UserApi m_userApi;
            FileApi m_fileApi;
            FileContentApi m_contentApi;
            FileLockApi m_lockApi;
            FileAclApi m_aclApi;
            FileVersionApi m_versionApi;
            GroupApi m_groupApi;
            NotificationStreamClient m_notificationStreamClient;

        public:

            explicit RemoteApiGateway(QUrl baseUrl);

            RemoteApiGateway(const RemoteApiGateway&) = delete;
            RemoteApiGateway& operator=(const RemoteApiGateway&) = delete;

            void setBaseUrl(QUrl baseUrl);
            void setBearerToken(QString token);
            void clearBearerToken();

            AuthApi& authApi() noexcept;
            UserApi& userApi() noexcept;
            FileApi& fileApi() noexcept;
            FileContentApi& contentApi() noexcept;
            FileLockApi& lockApi() noexcept;
            FileAclApi& aclApi() noexcept;
            FileVersionApi& versionApi() noexcept;
            GroupApi& groupApi() noexcept;

            void startNotifications(std::function<void(ApiResult<domain::models::NotificationEvent>)> callback);
            void stopNotifications();

    };

}
