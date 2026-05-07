#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>
#include <functional>

#include "api-result.h"
#include "domain/models/notification-event.h"

namespace client::infrastructure::api {

class NotificationStreamClient : public QObject {

        Q_OBJECT
        QUrl m_baseUrl;
        QString m_bearerToken;
        QNetworkAccessManager m_network;
        QNetworkReply* m_reply = nullptr;
        std::function<void(ApiResult<domain::models::NotificationEvent>)> m_callback;
        QByteArray m_buffer;
        QString m_currentEventName;
        QByteArray m_currentData;
        bool m_stopping = false;

    public:

        explicit NotificationStreamClient(QUrl baseUrl, QObject* parent = nullptr);
        ~NotificationStreamClient() override;

        NotificationStreamClient(const NotificationStreamClient&) = delete;
        NotificationStreamClient& operator=(const NotificationStreamClient&) = delete;

        void setBaseUrl(QUrl baseUrl);
        void setBearerToken(QString token);
        void clearBearerToken();

        void start(std::function<void(ApiResult<domain::models::NotificationEvent>)> callback);
        void stop();

    private:

        void consumeAvailableBytes();
        void consumeLine(QByteArray line);
        void dispatchCurrentEvent();
        void finishWithError(const QString& message, int httpStatus = 0);
        QNetworkRequest makeRequest() const;
};

}  // namespace client::infrastructure::api
