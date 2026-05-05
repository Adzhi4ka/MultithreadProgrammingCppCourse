#include "login-dialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include <utility>

namespace client::presentation {

    LoginDialog::LoginDialog(infrastructure::api::ApiClient& apiClient,
                             infrastructure::api::AuthApi& authApi,
                             QWidget* parent)
        : QDialog(parent),
          m_apiClient(apiClient),
          m_authApi(authApi) {
        buildUi();
    }

    domain::models::UserSession LoginDialog::session() const {
        return m_session;
    }

    void LoginDialog::buildUi() {
        setWindowTitle("File Storage Client: login");
        setMinimumWidth(420);

        m_baseUrlEdit = new QLineEdit(m_apiClient.baseUrl().isEmpty()
                                      ? QStringLiteral("http://127.0.0.1:8080")
                                      : m_apiClient.baseUrl().toString(), this);
        m_loginEdit = new QLineEdit(this);
        m_passwordEdit = new QLineEdit(this);
        m_passwordEdit->setEchoMode(QLineEdit::Password);

        auto* form = new QFormLayout;
        form->addRow("Server URL", m_baseUrlEdit);
        form->addRow("Login", m_loginEdit);
        form->addRow("Password", m_passwordEdit);

        m_loginButton = new QPushButton("Login", this);
        m_registerButton = new QPushButton("Register", this);
        m_statusLabel = new QLabel(this);
        m_statusLabel->setWordWrap(true);

        auto* buttons = new QHBoxLayout;
        buttons->addStretch();
        buttons->addWidget(m_registerButton);
        buttons->addWidget(m_loginButton);

        auto* root = new QVBoxLayout(this);
        root->addLayout(form);
        root->addWidget(m_statusLabel);
        root->addLayout(buttons);

        connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::startLogin);
        connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::startRegister);
        connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::startLogin);
    }

    void LoginDialog::startLogin() {
        authenticate(false);
    }

    void LoginDialog::startRegister() {
        authenticate(true);
    }

    void LoginDialog::authenticate(bool registration) {
        const auto baseUrl = QUrl::fromUserInput(m_baseUrlEdit->text().trimmed());
        const auto login = m_loginEdit->text().trimmed();
        const auto password = m_passwordEdit->text();

        if (!baseUrl.isValid() || baseUrl.scheme().isEmpty() || baseUrl.host().isEmpty()) {
            QMessageBox::warning(this, "Invalid server URL", "Enter valid server URL, for example http://127.0.0.1:8080");
            return;
        }

        if (login.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Invalid credentials", "Login and password are required");
            return;
        }

        m_apiClient.setBaseUrl(baseUrl);
        setBusy(true);
        m_statusLabel->setText(registration ? "Registering..." : "Logging in...");

        auto callback = [this](infrastructure::api::AuthApi::SessionResult result) mutable {
            setBusy(false);

            if (!result) {
                showError(result.error());
                return;
            }

            m_session = std::move(*result);
            m_apiClient.setBearerToken(m_session.token);
            accept();
        };

        if (registration) {
            m_authApi.registerUser(login, password, std::move(callback));
        } else {
            m_authApi.login(login, password, std::move(callback));
        }
    }

    void LoginDialog::setBusy(bool busy) {
        m_baseUrlEdit->setEnabled(!busy);
        m_loginEdit->setEnabled(!busy);
        m_passwordEdit->setEnabled(!busy);
        m_loginButton->setEnabled(!busy);
        m_registerButton->setEnabled(!busy);
    }

    void LoginDialog::showError(const infrastructure::api::ApiError& error) {
        const auto message = QStringLiteral("%1%2")
            .arg(error.httpStatus > 0 ? QStringLiteral("HTTP %1: ").arg(error.httpStatus) : QStringLiteral("Network error: "))
            .arg(error.message.isEmpty() ? QStringLiteral("unknown error") : error.message);
        m_statusLabel->setText(message);
        QMessageBox::warning(this, "Authentication failed", message);
    }

}
