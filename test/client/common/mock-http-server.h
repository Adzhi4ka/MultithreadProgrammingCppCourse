#pragma once

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QUrl>
#include <QUrlQuery>

#include <functional>
#include <vector>

namespace tests::client {

    struct MockHttpRequest {
        QByteArray method;
        QString path;
        QUrlQuery query;
        QHash<QByteArray, QByteArray> headers;
        QByteArray body;
    };

    struct MockHttpResponse {
        int status = 200;
        QByteArray contentType = "application/json; charset=utf-8";
        QByteArray body;
        QList<QPair<QByteArray, QByteArray>> headers;
    };

    class MockHttpServer : public QObject {

            struct Route {
                QByteArray method;
                QString path;
                std::function<MockHttpResponse(const MockHttpRequest&)> handler;
            };

            QTcpServer m_server;
            std::vector<Route> m_routes;
            QList<MockHttpRequest> m_requests;

        public:
            explicit MockHttpServer(QObject* parent = nullptr);

            bool start();
            void stop();

            QUrl baseUrl() const;
            qsizetype requestCount() const noexcept;
            QList<MockHttpRequest> requests() const;
            void clearRequests();

            void addRoute(QByteArray method,
                          QString path,
                          std::function<MockHttpResponse(const MockHttpRequest&)> handler);

        private:
            void handleIncomingConnection();
            MockHttpResponse dispatch(const MockHttpRequest& request) const;

            static bool tryReadRequest(const QByteArray& buffer, MockHttpRequest* request);
            static QByteArray serializeResponse(const MockHttpResponse& response);
            static QByteArray reasonPhrase(int status);

    };

}
