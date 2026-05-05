#pragma once

#include "domain/models/user-session.h"
#include "infrastructure/api/api-client.h"
#include "infrastructure/api/auth-api.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;

namespace client::presentation {

    class LoginDialog final : public QDialog {
        Q_OBJECT

    public:
        explicit LoginDialog(infrastructure::api::ApiClient& apiClient,
                             infrastructure::api::AuthApi& authApi,
                             QWidget* parent = nullptr);

        domain::models::UserSession session() const;

    private:
        void buildUi();
        void startLogin();
        void startRegister();
        void authenticate(bool registration);
        void setBusy(bool busy);
        void showError(const infrastructure::api::ApiError& error);

    private:
        infrastructure::api::ApiClient& m_apiClient;
        infrastructure::api::AuthApi& m_authApi;
        domain::models::UserSession m_session;

        QLineEdit* m_baseUrlEdit = nullptr;
        QLineEdit* m_loginEdit = nullptr;
        QLineEdit* m_passwordEdit = nullptr;
        QPushButton* m_loginButton = nullptr;
        QPushButton* m_registerButton = nullptr;
        QLabel* m_statusLabel = nullptr;
    };

}
