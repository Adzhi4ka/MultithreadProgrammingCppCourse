#include "api-client.h"

#include <QJsonDocument>
#include <QNetworkRequest>

namespace client::infrastructure::api {

ApiClient::ApiClient(QUrl baseUrl, QObject* parent) : QObject(parent), m_baseUrl(std::move(baseUrl)) {}

void ApiClient::setBaseUrl(QUrl baseUrl) { m_baseUrl = std::move(baseUrl); }

QUrl ApiClient::baseUrl() const { return m_baseUrl; }

void ApiClient::setBearerToken(QString token) { m_bearerToken = std::move(token); }

void ApiClient::clearBearerToken() { m_bearerToken.clear(); }

QString ApiClient::bearerToken() const { return m_bearerToken; }

void ApiClient::get(const QString& path, const QUrlQuery& query, std::function<void(RawApiResponse)> callback) {
    auto request = makeRequest(path, query);
    send(m_network.get(request), std::move(callback));
}

void ApiClient::postJson(const QString& path, const QJsonObject& body, std::function<void(RawApiResponse)> callback) {
    auto request = makeRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");

    const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);
    send(m_network.post(request, bytes), std::move(callback));
}

void ApiClient::putJson(const QString& path, const QJsonObject& body, std::function<void(RawApiResponse)> callback) {
    auto request = makeRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");

    const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);
    send(m_network.put(request, bytes), std::move(callback));
}

void ApiClient::putRaw(const QString& path, const QUrlQuery& query, const QByteArray& body,
                       const QByteArray& contentType, std::function<void(RawApiResponse)> callback) {

    auto request = makeRequest(path, query);
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    send(m_network.put(request, body), std::move(callback));
}

void ApiClient::deleteRequest(const QString& path, const QUrlQuery& query,
                              std::function<void(RawApiResponse)> callback) {

    auto request = makeRequest(path, query);
    send(m_network.deleteResource(request), std::move(callback));
}

void ApiClient::deleteJson(const QString& path, const QJsonObject& body, std::function<void(RawApiResponse)> callback) {

    auto request = makeRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");

    const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);
    send(m_network.sendCustomRequest(request, "DELETE", bytes), std::move(callback));
}

QNetworkRequest ApiClient::makeRequest(const QString& path, const QUrlQuery& query) const {

    auto normalizedPath = path;
    if (!normalizedPath.startsWith('/')) {
        normalizedPath.prepend('/');
    }

    auto url = m_baseUrl;
    url.setPath(normalizedPath);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json, application/octet-stream");

    if (!m_bearerToken.isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + m_bearerToken.toUtf8());
    }

    return request;
}

void ApiClient::send(QNetworkReply* reply, std::function<void(RawApiResponse)> callback) const {

    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, callback = std::move(callback)]() mutable {
        RawApiResponse response;
        response.networkError = reply->error();
        response.errorText = reply->errorString();
        response.body = reply->readAll();
        response.contentType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();

        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (status.isValid()) {
            response.httpStatus = status.toInt();
        }

        reply->deleteLater();

        if (callback) {
            callback(std::move(response));
        }
    });
}

}  // namespace client::infrastructure::api
