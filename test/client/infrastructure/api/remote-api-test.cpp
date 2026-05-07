#include <gtest/gtest.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

#include "common/mock-http-server.h"
#include "common/qt-test-helpers.h"
#include "infrastructure/api/api-client.h"
#include "infrastructure/api/auth-api.h"
#include "infrastructure/api/file-api.h"
#include "infrastructure/api/group-api.h"

namespace tests::client {

namespace api = ::client::infrastructure::api;
namespace models = ::client::domain::models;

TEST(ClientRemoteApiTest, AuthApi_Login_ParsesUserSession) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("POST", "/api/auth/login", [](const MockHttpRequest& request) {
        const auto object = QJsonDocument::fromJson(request.body).object();
        EXPECT_EQ(object.value(QStringLiteral("login")).toString(), QStringLiteral("alice"));
        EXPECT_EQ(object.value(QStringLiteral("password")).toString(), QStringLiteral("alice123"));

        return MockHttpResponse{.status = 200, .body = R"({"userId":7,"login":"alice","token":"token-7"})"};
    });

    api::ApiClient client{server.baseUrl()};
    api::AuthApi authApi{client};

    bool finished = false;
    ::client::ApiResult<models::UserSession> result;

    authApi.login(QStringLiteral("alice"), QStringLiteral("alice123"), [&](auto current) {
        result = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->userId, 7);
    EXPECT_EQ(result->login, QStringLiteral("alice"));
    EXPECT_EQ(result->token, QStringLiteral("token-7"));
}

TEST(ClientRemoteApiTest, AuthApi_Register_ReturnsErrorForInvalidJson) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("POST", "/api/auth/register",
                    [](const MockHttpRequest&) { return MockHttpResponse{.status = 200, .body = "not json"}; });

    api::ApiClient client{server.baseUrl()};
    api::AuthApi authApi{client};

    bool finished = false;
    ::client::ApiResult<models::UserSession> result;

    authApi.registerUser(QStringLiteral("bob"), QStringLiteral("bob123"), [&](auto current) {
        result = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, QStringLiteral("invalid_json"));
}

TEST(ClientRemoteApiTest, FileApi_GetAll_ParsesFileList) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("GET", "/api/files", [](const MockHttpRequest&) {
        return MockHttpResponse{
            .status = 200,
            .body =
                R"({"items":[{"id":10,"fullLogicalName":"/docs/a.txt","currentVersionId":100,"maxVersionCount":5,"createdAt":111,"createdBy":7},{"id":11,"fullLogicalName":"/docs/b.txt","currentVersionId":101,"maxVersionCount":3,"createdAt":112,"createdBy":7}]})"};
    });

    api::ApiClient client{server.baseUrl()};
    api::FileApi fileApi{client};

    bool finished = false;
    ::client::ApiResult<std::vector<models::RemoteFile>> result;

    fileApi.getAll([&](auto current) {
        result = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2u);
    EXPECT_EQ((*result)[0].id, 10);
    EXPECT_EQ((*result)[0].fullLogicalName, QStringLiteral("/docs/a.txt"));
    EXPECT_EQ((*result)[1].currentVersionId, 101);
}

TEST(ClientRemoteApiTest, FileApi_Create_SendsBodyAndParsesCreatedFile) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("POST", "/api/files", [](const MockHttpRequest& request) {
        const auto object = QJsonDocument::fromJson(request.body).object();
        EXPECT_EQ(object.value(QStringLiteral("logicalName")).toString(), QStringLiteral("/notes/todo.txt"));
        EXPECT_EQ((int)object.value(QStringLiteral("maxVersionCount")).toDouble(), 8);

        return MockHttpResponse{
            .status = 201,
            .body =
                R"({"id":12,"fullLogicalName":"/notes/todo.txt","currentVersionId":200,"maxVersionCount":8,"createdAt":120,"createdBy":7})"};
    });

    api::ApiClient client{server.baseUrl()};
    api::FileApi fileApi{client};

    bool finished = false;
    ::client::ApiResult<models::RemoteFile> result;

    fileApi.create(QStringLiteral("/notes/todo.txt"), 8, [&](auto current) {
        result = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 12);
    EXPECT_EQ(result->maxVersionCount, 8u);
}

TEST(ClientRemoteApiTest, FileApi_Remove_MapsServerError) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("DELETE", "/api/files", [](const MockHttpRequest& request) {
        EXPECT_EQ(request.query.queryItemValue(QStringLiteral("fileId")), QStringLiteral("42"));

        return MockHttpResponse{.status = 409, .body = R"({"error":"file_locked"})"};
    });

    api::ApiClient client{server.baseUrl()};
    api::FileApi fileApi{client};

    bool finished = false;
    ::client::ApiResult<void> result;

    fileApi.remove(42, [&](auto current) {
        result = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().httpStatus, 409);
    EXPECT_EQ(result.error().code, QStringLiteral("file_locked"));
    EXPECT_EQ(result.error().message, QStringLiteral("file_locked"));
}

TEST(ClientRemoteApiTest, GroupApi_GetUserGroups_ParsesIdList) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("GET", "/api/groups/by-user", [](const MockHttpRequest& request) {
        EXPECT_EQ(request.query.queryItemValue(QStringLiteral("userId")), QStringLiteral("7"));

        return MockHttpResponse{.status = 200, .body = R"({"items":[{"groupId":1},{"groupId":2}]})"};
    });

    api::ApiClient client{server.baseUrl()};
    api::GroupApi groupApi{client};

    bool finished = false;
    ::client::ApiResult<std::vector<qint64>> result;

    groupApi.getUserGroups(7, [&](auto current) {
        result = std::move(current);
        finished = true;
    });

    ASSERT_TRUE(waitUntil([&]() { return finished; }));
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2u);
    EXPECT_EQ((*result)[0], 1);
    EXPECT_EQ((*result)[1], 2);
}

}  // namespace tests::client
