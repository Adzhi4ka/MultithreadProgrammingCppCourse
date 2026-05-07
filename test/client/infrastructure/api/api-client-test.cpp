#include "infrastructure/api/api-client.h"

#include <gtest/gtest.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

#include "common/mock-http-server.h"
#include "common/qt-test-helpers.h"

namespace tests::client {

namespace api = ::client::infrastructure::api;

TEST(ClientApiClientTest, Get_SendsBearerTokenAndParsesRawResponse) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("GET", "/api/files", [](const MockHttpRequest& request) {
        EXPECT_EQ(request.headers.value("authorization"), "Bearer test-token");
        EXPECT_EQ(request.query.queryItemValue(QStringLiteral("page")), QStringLiteral("1"));

        return MockHttpResponse{.status = 200, .body = R"({"items":[]})"};
    });

    api::ApiClient client{server.baseUrl()};
    client.setBearerToken(QStringLiteral("test-token"));

    bool finished = false;
    api::RawApiResponse response;

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QStringLiteral("1"));

    client.get(QStringLiteral("/api/files"), query, [&](api::RawApiResponse current) {
        response = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    EXPECT_EQ(response.httpStatus, 200);
    EXPECT_EQ(response.body, R"({"items":[]})");
    EXPECT_EQ(server.requestCount(), 1);
}

TEST(ClientApiClientTest, PostJson_SendsJsonBodyAndContentType) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("POST", "/api/auth/login", [](const MockHttpRequest& request) {
        EXPECT_TRUE(request.headers.value("content-type").startsWith("application/json"));

        const auto object = QJsonDocument::fromJson(request.body).object();
        EXPECT_EQ(object.value(QStringLiteral("login")).toString(), QStringLiteral("alice"));
        EXPECT_EQ(object.value(QStringLiteral("password")).toString(), QStringLiteral("alice123"));

        return MockHttpResponse{.status = 200, .body = R"({"ok":true})"};
    });

    api::ApiClient client{server.baseUrl()};

    bool finished = false;
    api::RawApiResponse response;

    client.postJson(QStringLiteral("/api/auth/login"), QJsonObject{{"login", "alice"}, {"password", "alice123"}},
                    [&](api::RawApiResponse current) {
                        response = std::move(current);
                        finished = true;
                    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    EXPECT_EQ(response.httpStatus, 200);
    EXPECT_EQ(server.requestCount(), 1);
}

}  // namespace tests::client
