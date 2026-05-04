#pragma once

#include "api-result.h"

#include <QByteArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <QUrlQuery>

#include <functional>

namespace client::infrastructure::api {

    struct RawApiResponse {
        int httpStatus = 0;
        QByteArray body;
        QByteArray contentType;
        QNetworkReply::NetworkError networkError = QNetworkReply::NoError;
        QString errorText;

        bool isSuccessStatus() const noexcept {
            return httpStatus >= 200 && httpStatus < 300;
        }
    };

    using RawApiCallback = std::function<void(RawApiResponse)>;

    class ApiClient final : public QObject {
        Q_OBJECT

    public:
        explicit ApiClient(QUrl baseUrl, QObject* parent = nullptr);

        void setBaseUrl(QUrl baseUrl);
        QUrl baseUrl() const;

        void setBearerToken(QString token);
        void clearBearerToken();
        QString bearerToken() const;

        void get(const QString& path, const QUrlQuery& query, RawApiCallback callback);
        void postJson(const QString& path, const QJsonObject& body, RawApiCallback callback);
        void putJson(const QString& path, const QJsonObject& body, RawApiCallback callback);
        void putRaw(const QString& path,
                    const QUrlQuery& query,
                    const QByteArray& body,
                    const QByteArray& contentType,
                    RawApiCallback callback);
        void deleteRequest(const QString& path, const QUrlQuery& query, RawApiCallback callback);
        void deleteJson(const QString& path, const QJsonObject& body, RawApiCallback callback);

    private:
        QNetworkRequest makeRequest(const QString& path, const QUrlQuery& query = {}) const;
        void send(QNetworkReply* reply, RawApiCallback callback) const;

    private:
        QUrl m_baseUrl;
        QString m_bearerToken;
        QNetworkAccessManager m_network;
    };

}
