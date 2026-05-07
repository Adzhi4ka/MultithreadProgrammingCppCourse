#include "presentation/editor-window.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QLabel>
#include <QPushButton>
#include <QTest>
#include <QTextEdit>

#include "application/client-runtime.h"
#include "common/mock-http-server.h"
#include "common/qt-test-helpers.h"

namespace tests::client {

namespace app = ::client::application;
namespace ui = ::client::presentation;

TEST(ClientUiTest, EditorWindow_SaveButton_UploadsContentAndShowsSavedVersion) {
    MockHttpServer server;
    ASSERT_TRUE(server.start());

    server.addRoute("PUT", "/api/files/content", [](const MockHttpRequest& request) {
        EXPECT_EQ(request.query.queryItemValue(QStringLiteral("fileId")), QStringLiteral("42"));
        EXPECT_EQ(request.body, "updated content");

        return MockHttpResponse{
            .status = 200,
            .body = R"({"id":300,"fileId":42,"version":2,"logicalNameSnapshot":"/docs/a.txt","createdAt":123})"};
    });

    server.addRoute("DELETE", "/api/file-locks", [](const MockHttpRequest& request) {
        EXPECT_EQ(request.query.queryItemValue(QStringLiteral("fileId")), QStringLiteral("42"));
        EXPECT_EQ(request.query.queryItemValue(QStringLiteral("lockToken")), QStringLiteral("100500"));

        return MockHttpResponse{.status = 204, .body = QByteArray{}};
    });

    app::ClientRuntime runtime{server.baseUrl()};
    auto* window =
        ui::EditorWindow::createEditable(runtime, 42, 100500, QStringLiteral("/docs/a.txt"), QByteArray{"old content"});
    window->show();

    auto* editor = window->findChild<QTextEdit*>(QStringLiteral("editor"));
    auto* saveButton = window->findChild<QPushButton*>(QStringLiteral("saveButton"));
    auto* releaseButton = window->findChild<QPushButton*>(QStringLiteral("releaseButton"));
    auto* statusLabel = window->findChild<QLabel*>(QStringLiteral("statusLabel"));

    ASSERT_NE(editor, nullptr);
    ASSERT_NE(saveButton, nullptr);
    ASSERT_NE(releaseButton, nullptr);
    ASSERT_NE(statusLabel, nullptr);
    ASSERT_FALSE(editor->isReadOnly());
    ASSERT_TRUE(saveButton->isEnabled());

    editor->setPlainText(QStringLiteral("updated content"));
    QTest::mouseClick(saveButton, Qt::LeftButton);

    ASSERT_TRUE(waitUntil([&]() { return statusLabel->text().contains(QStringLiteral("Saved version 2")); }, 5000));
    QTest::mouseClick(releaseButton, Qt::LeftButton);
    ASSERT_TRUE(waitUntil([&]() { return statusLabel->text().contains(QStringLiteral("Lock released")); }, 5000));
    EXPECT_EQ(server.requestCount(), 2);

    window->close();
}

}  // namespace tests::client
