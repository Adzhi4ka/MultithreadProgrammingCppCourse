#include "remote-api-gateway.h"

#include <utility>

namespace client::infrastructure::api {

RemoteApiGateway::RemoteApiGateway(QUrl baseUrl)
    : m_apiClient(std::move(baseUrl)),
      m_authApi(m_apiClient),
      m_userApi(m_apiClient),
      m_fileApi(m_apiClient),
      m_contentApi(m_apiClient),
      m_lockApi(m_apiClient),
      m_aclApi(m_apiClient),
      m_versionApi(m_apiClient),
      m_groupApi(m_apiClient),
      m_notificationStreamClient(m_apiClient.baseUrl()) {}

void RemoteApiGateway::setBaseUrl(QUrl baseUrl) {
    m_apiClient.setBaseUrl(baseUrl);
    m_notificationStreamClient.setBaseUrl(std::move(baseUrl));
}

void RemoteApiGateway::setBearerToken(QString token) {
    m_apiClient.setBearerToken(token);
    m_notificationStreamClient.setBearerToken(std::move(token));
}

void RemoteApiGateway::clearBearerToken() {
    m_apiClient.clearBearerToken();
    m_notificationStreamClient.clearBearerToken();
    m_notificationStreamClient.stop();
}

AuthApi& RemoteApiGateway::authApi() noexcept { return m_authApi; }

UserApi& RemoteApiGateway::userApi() noexcept { return m_userApi; }

FileApi& RemoteApiGateway::fileApi() noexcept { return m_fileApi; }

FileContentApi& RemoteApiGateway::contentApi() noexcept { return m_contentApi; }

FileLockApi& RemoteApiGateway::lockApi() noexcept { return m_lockApi; }

FileAclApi& RemoteApiGateway::aclApi() noexcept { return m_aclApi; }

FileVersionApi& RemoteApiGateway::versionApi() noexcept { return m_versionApi; }

GroupApi& RemoteApiGateway::groupApi() noexcept { return m_groupApi; }

void RemoteApiGateway::startNotifications(std::function<void(ApiResult<domain::models::NotificationEvent>)> callback) {
    m_notificationStreamClient.start(std::move(callback));
}

void RemoteApiGateway::stopNotifications() { m_notificationStreamClient.stop(); }

}  // namespace client::infrastructure::api
