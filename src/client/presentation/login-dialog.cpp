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

LoginDialog::LoginDialog(application::ClientRuntime& runtime, QWidget* parent) : QDialog(parent), m_runtime(runtime) {
    buildUi();
    connect(&m_runtime, &application::ClientRuntime::authenticationFinished, this,
            &LoginDialog::handleAuthenticationFinished);
}

domain::models::UserSession LoginDialog::session() const { return m_session; }

void LoginDialog::buildUi() {
    setWindowTitle(QStringLiteral("File Storage Client: login"));
    setMinimumWidth(420);

    m_baseUrlEdit = new QLineEdit(QStringLiteral("http://127.0.0.1:8080"), this);
    m_baseUrlEdit->setObjectName(QStringLiteral("baseUrlEdit"));
    m_loginEdit = new QLineEdit(this);
    m_loginEdit->setObjectName(QStringLiteral("loginEdit"));
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setObjectName(QStringLiteral("passwordEdit"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Server URL"), m_baseUrlEdit);
    form->addRow(QStringLiteral("Login"), m_loginEdit);
    form->addRow(QStringLiteral("Password"), m_passwordEdit);

    m_loginButton = new QPushButton(QStringLiteral("Login"), this);
    m_loginButton->setObjectName(QStringLiteral("loginButton"));
    m_registerButton = new QPushButton(QStringLiteral("Register"), this);
    m_registerButton->setObjectName(QStringLiteral("registerButton"));
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
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

void LoginDialog::startLogin() { authenticate(false); }

void LoginDialog::startRegister() { authenticate(true); }

void LoginDialog::authenticate(bool registration) {
    const auto baseUrl = QUrl::fromUserInput(m_baseUrlEdit->text().trimmed());
    const auto login = m_loginEdit->text().trimmed();
    const auto password = m_passwordEdit->text();

    if (!baseUrl.isValid() || baseUrl.scheme().isEmpty() || baseUrl.host().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid server URL"),
                             QStringLiteral("Enter valid server URL, for example http://127.0.0.1:8080"));
        return;
    }

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid credentials"),
                             QStringLiteral("Login and password are required"));
        return;
    }

    setBusy(true);
    m_statusLabel->setText(registration ? QStringLiteral("Registering...") : QStringLiteral("Logging in..."));
    m_runtime.authenticate(baseUrl, login, password, registration);
}

void LoginDialog::handleAuthenticationFinished(ApiResult<domain::models::UserSession> result) {
    setBusy(false);

    if (!result) {
        showError(result.error());
        return;
    }

    m_session = std::move(*result);
    accept();
}

void LoginDialog::setBusy(bool busy) {
    m_baseUrlEdit->setEnabled(!busy);
    m_loginEdit->setEnabled(!busy);
    m_passwordEdit->setEnabled(!busy);
    m_loginButton->setEnabled(!busy);
    m_registerButton->setEnabled(!busy);
}

void LoginDialog::showError(const ApiError& error) {
    const auto message = QStringLiteral("%1%2")
                             .arg(error.httpStatus > 0 ? QStringLiteral("HTTP %1: ").arg(error.httpStatus)
                                                       : QStringLiteral("Network error: "))
                             .arg(error.message.isEmpty() ? QStringLiteral("unknown error") : error.message);
    m_statusLabel->setText(message);
    QMessageBox::warning(this, QStringLiteral("Authentication failed"), message);
}

}  // namespace client::presentation
