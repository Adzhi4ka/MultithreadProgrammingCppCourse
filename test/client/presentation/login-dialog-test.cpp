#include "application/client-runtime.h"
#include "common/mock-http-server.h"
#include "common/qt-test-helpers.h"
#include "presentation/login-dialog.h"

#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

namespace tests::client {

    namespace app = ::client::application;
    namespace ui = ::client::presentation;

    TEST(ClientUiTest, LoginDialog_LoginButton_AuthenticatesAndAcceptsDialog) {
        MockHttpServer server;
        ASSERT_TRUE(server.start());

        server.addRoute("POST", "/api/auth/login", [](const MockHttpRequest& request) {
            const auto object = QJsonDocument::fromJson(request.body).object();
            EXPECT_EQ(object.value(QStringLiteral("login")).toString(), QStringLiteral("alice"));
            EXPECT_EQ(object.value(QStringLiteral("password")).toString(), QStringLiteral("alice123"));

            return MockHttpResponse{
                .status = 200,
                .body = R"({"userId":7,"login":"alice","token":"token-7"})"
            };
        });

        app::ClientRuntime runtime{server.baseUrl()};
        ui::LoginDialog dialog{runtime};
        dialog.show();

        auto* baseUrlEdit = dialog.findChild<QLineEdit*>(QStringLiteral("baseUrlEdit"));
        auto* loginEdit = dialog.findChild<QLineEdit*>(QStringLiteral("loginEdit"));
        auto* passwordEdit = dialog.findChild<QLineEdit*>(QStringLiteral("passwordEdit"));
        auto* loginButton = dialog.findChild<QPushButton*>(QStringLiteral("loginButton"));

        ASSERT_NE(baseUrlEdit, nullptr);
        ASSERT_NE(loginEdit, nullptr);
        ASSERT_NE(passwordEdit, nullptr);
        ASSERT_NE(loginButton, nullptr);

        baseUrlEdit->setText(server.baseUrl().toString());
        loginEdit->setText(QStringLiteral("alice"));
        passwordEdit->setText(QStringLiteral("alice123"));

        QSignalSpy finishedSpy{&dialog, &QDialog::finished};
        QTest::mouseClick(loginButton, Qt::LeftButton);

        ASSERT_TRUE(waitUntil([&]() { return dialog.result() == QDialog::Accepted; }));
        ASSERT_GE(finishedSpy.count(), 1);

        const auto session = dialog.session();
        EXPECT_EQ(session.userId, 7);
        EXPECT_EQ(session.login, QStringLiteral("alice"));
        EXPECT_EQ(session.token, QStringLiteral("token-7"));
        EXPECT_EQ(server.requestCount(), 1);
    }

} // namespace tests::client
