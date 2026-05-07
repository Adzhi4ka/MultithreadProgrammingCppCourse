#include "presentation/main-window.h"

#include <gtest/gtest.h>

#include <QJsonObject>
#include <QLabel>
#include <QTest>
#include <QTreeWidgetItem>
#include <memory>

#include "application/client-runtime.h"
#include "common/mock-http-server.h"
#include "common/qt-test-helpers.h"
#include "domain/models/notification-event.h"
#include "domain/models/user-session.h"
#include "presentation/widgets/file-tree-widget.h"

namespace tests::client {

namespace app = ::client::application;
namespace models = ::client::domain::models;
namespace ui = ::client::presentation;

namespace {

models::UserSession makeSession() {
    return models::UserSession{
        .userId = 7,
        .login = QStringLiteral("alice"),
        .token = QStringLiteral("token-7"),
    };
}

void addConnectedNotificationStream(MockHttpServer& server) {
    server.addRoute("GET", "/api/notifications/stream", [](const MockHttpRequest&) {
        return MockHttpResponse{.status = 200,
                                .contentType = "text/event-stream; charset=utf-8",
                                .body = "event: connected\ndata: {\"userId\":7}\n\n"};
    });
}

void addReadWriteAclRoute(MockHttpServer& server) {
    server.addRoute("GET", "/api/file-acl/users", [](const MockHttpRequest& request) {
        return MockHttpResponse{.status = 200,
                                .body = QByteArray{"{\"fileId\":"} +
                                        request.query.queryItemValue(QStringLiteral("fileId")).toUtf8() +
                                        ",\"userId\":7,\"aclLevel\":\"read_write\"}"};
    });
}

QTreeWidgetItem* findFileItem(ui::FileTreeWidget& fileTree, const QString& fileName) {
    const auto items = fileTree.findItems(fileName, Qt::MatchRecursive, 0);
    return items.empty() ? nullptr : items.front();
}

}  // namespace

TEST(ClientUiTest, MainWindow_Startup_LoadsFilesThroughRuntimeSignal) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    addConnectedNotificationStream(server);

    server.addRoute("GET", "/api/files", [](const MockHttpRequest&) {
        return MockHttpResponse{
            .status = 200,
            .body =
                R"({"items":[{"id":10,"fullLogicalName":"/docs/a.txt","currentVersionId":100,"maxVersionCount":5,"createdAt":111,"createdBy":7},{"id":11,"fullLogicalName":"/docs/b.txt","currentVersionId":101,"maxVersionCount":3,"createdAt":112,"createdBy":7}]})"};
    });

    addReadWriteAclRoute(server);

    server.addRoute("GET", "/api/file-locks/active", [](const MockHttpRequest&) {
        return MockHttpResponse{.status = 404, .body = R"({"error":"not_found"})"};
    });

    app::ClientRuntime runtime{server.baseUrl()};
    const auto session = makeSession();
    runtime.setSession(session);

    ui::MainWindow window{runtime, session};
    window.show();

    auto* fileTree = window.findChild<ui::FileTreeWidget*>(QStringLiteral("fileTree"));
    auto* statusLabel = window.findChild<QLabel*>(QStringLiteral("statusLabel"));

    ASSERT_NE(fileTree, nullptr);
    ASSERT_NE(statusLabel, nullptr);
    ASSERT_TRUE(waitUntil([&]() { return fileTree->fileCount() == 2; }, 5000));

    EXPECT_EQ(fileTree->fileCount(), 2);
    EXPECT_TRUE(statusLabel->text().contains(QStringLiteral("Loaded 2 files")));

    const auto firstFile = fileTree->fileById(10);
    ASSERT_TRUE(firstFile.has_value());
    EXPECT_EQ(firstFile->aclLevel, models::AclLevel::ReadWrite);
}

TEST(ClientUiTest, MainWindow_FileLockedNotification_RefreshesFilesAndShowsLockInGui) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    auto lockActive = std::make_shared<bool>(false);

    addConnectedNotificationStream(server);

    server.addRoute("GET", "/api/files", [](const MockHttpRequest&) {
        return MockHttpResponse{
            .status = 200,
            .body =
                R"({"items":[{"id":42,"fullLogicalName":"/docs/report.txt","currentVersionId":100,"maxVersionCount":5,"createdAt":111,"createdBy":7}]})"};
    });

    addReadWriteAclRoute(server);

    server.addRoute("GET", "/api/file-locks/active", [lockActive](const MockHttpRequest&) {
        if (!*lockActive) {
            return MockHttpResponse{.status = 404, .body = R"({"error":"not_found"})"};
        }

        return MockHttpResponse{.status = 200,
                                .body = R"({"fileId":42,"userId":55,"leaseUntil":222222,"lockToken":999})"};
    });

    app::ClientRuntime runtime{server.baseUrl()};
    const auto session = makeSession();
    runtime.setSession(session);

    ui::MainWindow window{runtime, session};
    window.show();

    auto* fileTree = window.findChild<ui::FileTreeWidget*>(QStringLiteral("fileTree"));
    ASSERT_NE(fileTree, nullptr);

    ASSERT_TRUE(waitUntil(
        [&]() {
            const auto file = fileTree->fileById(42);
            return file.has_value() && !file->hasActiveLock;
        },
        5000));

    auto* initialItem = findFileItem(*fileTree, QStringLiteral("report.txt"));
    ASSERT_NE(initialItem, nullptr);
    EXPECT_EQ(initialItem->text(2), QStringLiteral("free"));

    *lockActive = true;

    emit runtime.notificationReceived(::client::apiSuccess(models::NotificationEvent{
        .name = QStringLiteral("file_locked"),
        .payload =
            QJsonObject{
                {"fileId", 42},
                {"userId", 55},
                {"leaseUntil", 222222},
            },
    }));

    ASSERT_TRUE(waitUntil(
        [&]() {
            const auto file = fileTree->fileById(42);
            return file.has_value() && file->hasActiveLock && file->lockedByUserId.value_or(0) == 55;
        },
        5000));

    auto* lockedItem = findFileItem(*fileTree, QStringLiteral("report.txt"));
    ASSERT_NE(lockedItem, nullptr);
    EXPECT_EQ(lockedItem->text(2), QStringLiteral("locked by unknown user"));

    const auto lockedFile = fileTree->fileById(42);
    ASSERT_TRUE(lockedFile.has_value());
    EXPECT_TRUE(lockedFile->hasActiveLock);
    EXPECT_EQ(lockedFile->lockedByUserId.value_or(0), 55);
    EXPECT_EQ(lockedFile->lockLeaseUntil.value_or(0), 222222);
}

TEST(ClientUiTest, MainWindow_GroupAssignedNotification_ReloadsFilesAndShowsNewFileInGui) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    auto showSharedFile = std::make_shared<bool>(false);

    addConnectedNotificationStream(server);

    server.addRoute("GET", "/api/files", [showSharedFile](const MockHttpRequest&) {
        if (!*showSharedFile) {
            return MockHttpResponse{
                .status = 200,
                .body =
                    R"({"items":[{"id":10,"fullLogicalName":"/docs/a.txt","currentVersionId":100,"maxVersionCount":5,"createdAt":111,"createdBy":7}]})"};
        }

        return MockHttpResponse{
            .status = 200,
            .body =
                R"({"items":[{"id":10,"fullLogicalName":"/docs/a.txt","currentVersionId":100,"maxVersionCount":5,"createdAt":111,"createdBy":7},{"id":20,"fullLogicalName":"/shared/new-role-file.txt","currentVersionId":200,"maxVersionCount":3,"createdAt":222,"createdBy":8}]})"};
    });

    addReadWriteAclRoute(server);

    server.addRoute("GET", "/api/file-locks/active", [](const MockHttpRequest&) {
        return MockHttpResponse{.status = 404, .body = R"({"error":"not_found"})"};
    });

    app::ClientRuntime runtime{server.baseUrl()};
    const auto session = makeSession();
    runtime.setSession(session);

    ui::MainWindow window{runtime, session};
    window.show();

    auto* fileTree = window.findChild<ui::FileTreeWidget*>(QStringLiteral("fileTree"));
    auto* statusLabel = window.findChild<QLabel*>(QStringLiteral("statusLabel"));

    ASSERT_NE(fileTree, nullptr);
    ASSERT_NE(statusLabel, nullptr);

    ASSERT_TRUE(waitUntil([&]() { return fileTree->fileCount() == 1 && fileTree->fileById(10).has_value(); }, 5000));

    EXPECT_FALSE(fileTree->fileById(20).has_value());

    *showSharedFile = true;

    emit runtime.notificationReceived(::client::apiSuccess(models::NotificationEvent{
        .name = QStringLiteral("group_assigned"),
        .payload =
            QJsonObject{
                {"groupId", 5},
                {"userId", 7},
            },
    }));

    ASSERT_TRUE(waitUntil([&]() { return fileTree->fileCount() == 2 && fileTree->fileById(20).has_value(); }, 5000));

    EXPECT_TRUE(statusLabel->text().contains(QStringLiteral("Loaded 2 files")));

    auto* sharedItem = findFileItem(*fileTree, QStringLiteral("new-role-file.txt"));
    ASSERT_NE(sharedItem, nullptr);
    EXPECT_EQ(sharedItem->text(1), QStringLiteral("read/write"));

    const auto sharedFile = fileTree->fileById(20);
    ASSERT_TRUE(sharedFile.has_value());
    EXPECT_EQ(sharedFile->fullLogicalName, QStringLiteral("/shared/new-role-file.txt"));
    EXPECT_EQ(sharedFile->aclLevel, models::AclLevel::ReadWrite);
}

}  // namespace tests::client