#include "notification-stream-client.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <utility>

namespace client::infrastructure::api {

NotificationStreamClient::NotificationStreamClient(QUrl baseUrl, QObject* parent)
    : QObject(parent), m_baseUrl(std::move(baseUrl)) {}

NotificationStreamClient::~NotificationStreamClient() { stop(); }

void NotificationStreamClient::setBaseUrl(QUrl baseUrl) { m_baseUrl = std::move(baseUrl); }

void NotificationStreamClient::setBearerToken(QString token) { m_bearerToken = std::move(token); }

void NotificationStreamClient::clearBearerToken() { m_bearerToken.clear(); }

void NotificationStreamClient::start(std::function<void(ApiResult<domain::models::NotificationEvent>)> callback) {
    stop();

    m_callback = std::move(callback);
    m_buffer.clear();
    m_currentEventName.clear();
    m_currentData.clear();
    m_stopping = false;

    m_reply = m_network.get(makeRequest());

    connect(m_reply, &QNetworkReply::readyRead, this, &NotificationStreamClient::consumeAvailableBytes);
    connect(m_reply, &QNetworkReply::finished, this, [this]() {
        auto* reply = m_reply;
        if (!reply) {
            return;
        }

        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const auto httpStatus = status.isValid() ? status.toInt() : 0;
        const auto errorText = reply->errorString();
        const auto hasError = reply->error() != QNetworkReply::NoError || (httpStatus != 0 && httpStatus >= 400);

        reply->deleteLater();
        m_reply = nullptr;

        if (!m_stopping && hasError) {
            finishWithError(errorText, httpStatus);
        }
    });
}

void NotificationStreamClient::stop() {
    m_stopping = true;
    if (m_reply) {
        auto* reply = m_reply;
        m_reply = nullptr;
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }

    m_buffer.clear();
    m_currentEventName.clear();
    m_currentData.clear();
}

void NotificationStreamClient::consumeAvailableBytes() {
    if (!m_reply) {
        return;
    }

    m_buffer += m_reply->readAll();

    while (true) {
        const auto newlineIndex = m_buffer.indexOf('\n');
        if (newlineIndex < 0) {
            break;
        }

        auto line = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);
        if (line.endsWith('\r')) {
            line.chop(1);
        }

        consumeLine(std::move(line));
    }
}

void NotificationStreamClient::consumeLine(QByteArray line) {
    if (line.isEmpty()) {
        dispatchCurrentEvent();
        return;
    }

    if (line.startsWith(':')) {
        return;
    }

    if (line.startsWith("event:")) {
        m_currentEventName = QString::fromUtf8(line.mid(6).trimmed());
        return;
    }

    if (line.startsWith("data:")) {
        if (!m_currentData.isEmpty()) {
            m_currentData += '\n';
        }
        m_currentData += line.mid(5).trimmed();
    }
}

void NotificationStreamClient::dispatchCurrentEvent() {
    if (m_currentEventName.isEmpty() && m_currentData.isEmpty()) {
        return;
    }

    const auto json = QJsonDocument::fromJson(m_currentData);
    if (!json.isObject()) {
        m_currentEventName.clear();
        m_currentData.clear();
        return;
    }

    domain::models::NotificationEvent event{.name = m_currentEventName, .payload = json.object()};

    if (m_callback) {
        m_callback(apiSuccess(std::move(event)));
    }

    m_currentEventName.clear();
    m_currentData.clear();
}

void NotificationStreamClient::finishWithError(const QString& message, int httpStatus) {
    if (!m_callback) {
        return;
    }

    m_callback(apiFailure({.httpStatus = httpStatus,
                           .code = QStringLiteral("notification_stream_error"),
                           .message = message.isEmpty() ? QStringLiteral("notification stream closed") : message}));
}

QNetworkRequest NotificationStreamClient::makeRequest() const {
    auto url = m_baseUrl;
    url.setPath(QStringLiteral("/api/notifications/stream"));
    url.setQuery(QUrlQuery{});

    QNetworkRequest request{url};
    request.setRawHeader("Accept", "text/event-stream");
    request.setRawHeader("Cache-Control", "no-cache");

    if (!m_bearerToken.isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + m_bearerToken.toUtf8());
    }

    return request;
}

}  // namespace client::infrastructure::api
