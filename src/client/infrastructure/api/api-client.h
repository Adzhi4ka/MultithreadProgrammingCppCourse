#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <QUrlQuery>
#include <functional>

#include "api-result.h"

namespace client::infrastructure::api {

struct RawApiResponse {
        int httpStatus = 0;
        QByteArray body;
        QByteArray contentType;
        QNetworkReply::NetworkError networkError = QNetworkReply::NoError;
        QString errorText;

        bool isSuccessStatus() const noexcept { return httpStatus >= 200 && httpStatus < 300; }
};

class ApiClient : public QObject {

        Q_OBJECT

        QUrl m_baseUrl;
        QString m_bearerToken;
        QNetworkAccessManager m_network;

    public:

        explicit ApiClient(QUrl baseUrl, QObject* parent = nullptr);

        void setBaseUrl(QUrl baseUrl);
        QUrl baseUrl() const;

        void setBearerToken(QString token);
        void clearBearerToken();
        QString bearerToken() const;

        void get(const QString& path, const QUrlQuery& query, std::function<void(RawApiResponse)> callback);
        void postJson(const QString& path, const QJsonObject& body, std::function<void(RawApiResponse)> callback);
        void putJson(const QString& path, const QJsonObject& body, std::function<void(RawApiResponse)> callback);
        void putRaw(const QString& path, const QUrlQuery& query, const QByteArray& body, const QByteArray& contentType,
                    std::function<void(RawApiResponse)> callback);
        void deleteRequest(const QString& path, const QUrlQuery& query, std::function<void(RawApiResponse)> callback);
        void deleteJson(const QString& path, const QJsonObject& body, std::function<void(RawApiResponse)> callback);

    private:

        QNetworkRequest makeRequest(const QString& path, const QUrlQuery& query = {}) const;
        void send(QNetworkReply* reply, std::function<void(RawApiResponse)> callback) const;
};

}  // namespace client::infrastructure::api
