#include "mock-http-server.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <algorithm>
#include <memory>
#include <utility>

namespace tests::client {

namespace {

QByteArray normalizedHeaderName(QByteArray name) { return name.trimmed().toLower(); }

int contentLengthFromHeaders(const QHash<QByteArray, QByteArray>& headers) {
    const auto it = headers.find("content-length");
    if (it == headers.end()) {
        return 0;
    }

    bool ok = false;
    const auto length = it->trimmed().toInt(&ok);
    return ok && length > 0 ? length : 0;
}

}  // namespace

MockHttpServer::MockHttpServer(QObject* parent) : QObject(parent) {
    connect(&m_server, &QTcpServer::newConnection, this, &MockHttpServer::handleIncomingConnection);
}

bool MockHttpServer::start() { return m_server.listen(QHostAddress::LocalHost, 0); }

void MockHttpServer::stop() { m_server.close(); }

QUrl MockHttpServer::baseUrl() const {
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort((int)m_server.serverPort());
    return url;
}

qsizetype MockHttpServer::requestCount() const noexcept { return m_requests.size(); }

QList<MockHttpRequest> MockHttpServer::requests() const { return m_requests; }

void MockHttpServer::clearRequests() { m_requests.clear(); }

void MockHttpServer::addRoute(QByteArray method, QString path,
                              std::function<MockHttpResponse(const MockHttpRequest&)> handler) {
    m_routes.push_back(Route{std::move(method), std::move(path), std::move(handler)});
}

void MockHttpServer::handleIncomingConnection() {
    while (auto* socket = m_server.nextPendingConnection()) {
        socket->setParent(this);
        auto buffer = std::make_shared<QByteArray>();
        auto processed = std::make_shared<bool>(false);

        connect(socket, &QTcpSocket::readyRead, this, [this, socket, buffer, processed]() {
            if (*processed) {
                return;
            }

            buffer->append(socket->readAll());

            MockHttpRequest request;
            if (!tryReadRequest(*buffer, &request)) {
                return;
            }

            *processed = true;
            m_requests.push_back(request);

            const auto response = dispatch(request);
            socket->write(serializeResponse(response));
            socket->flush();
            socket->disconnectFromHost();
        });

        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

MockHttpResponse MockHttpServer::dispatch(const MockHttpRequest& request) const {
    const auto it = std::find_if(m_routes.begin(), m_routes.end(), [&request](const Route& route) {
        return route.method == request.method && route.path == request.path;
    });

    if (it != m_routes.end()) {
        return it->handler(request);
    }

    return MockHttpResponse{
        .status = 404, .contentType = "application/json; charset=utf-8", .body = R"({"error":"not_found"})"};
}

bool MockHttpServer::tryReadRequest(const QByteArray& buffer, MockHttpRequest* request) {
    const auto headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        return false;
    }

    const auto headerBytes = buffer.left(headerEnd);
    const auto lines = headerBytes.split('\n');
    if (lines.empty()) {
        return false;
    }

    const auto requestLine = lines.front().trimmed();
    const auto requestParts = requestLine.split(' ');
    if (requestParts.size() < 2) {
        return false;
    }

    MockHttpRequest parsed;
    parsed.method = requestParts[0].trimmed().toUpper();

    const auto target = QString::fromUtf8(requestParts[1]);
    const QUrl url{QStringLiteral("http://mock") + target};
    parsed.path = url.path();
    parsed.query = QUrlQuery{url};

    for (qsizetype i = 1; i < lines.size(); ++i) {
        const auto line = lines[i].trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const auto colon = line.indexOf(':');
        if (colon <= 0) {
            continue;
        }

        const auto name = normalizedHeaderName(line.left(colon));
        const auto value = line.mid(colon + 1).trimmed();
        parsed.headers.insert(name, value);
    }

    const auto bodyOffset = headerEnd + 4;
    const auto contentLength = contentLengthFromHeaders(parsed.headers);
    if (buffer.size() < bodyOffset + contentLength) {
        return false;
    }

    parsed.body = buffer.mid(bodyOffset, contentLength);
    *request = std::move(parsed);
    return true;
}

QByteArray MockHttpServer::serializeResponse(const MockHttpResponse& response) {
    QByteArray bytes;
    bytes += "HTTP/1.1 " + QByteArray::number(response.status) + ' ' + reasonPhrase(response.status) + "\r\n";
    bytes += "Content-Length: " + QByteArray::number(response.body.size()) + "\r\n";
    bytes += "Content-Type: " + response.contentType + "\r\n";
    bytes += "Connection: close\r\n";

    for (const auto& header : response.headers) {
        bytes += header.first + ": " + header.second + "\r\n";
    }

    bytes += "\r\n";
    bytes += response.body;
    return bytes;
}

QByteArray MockHttpServer::reasonPhrase(int status) {
    switch (status) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 409:
            return "Conflict";
        case 500:
            return "Internal Server Error";
        default:
            return "Status";
    }
}

}  // namespace tests::client
