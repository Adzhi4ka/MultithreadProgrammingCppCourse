#pragma once

#include <QDialog>

#include "application/client-runtime.h"
#include "domain/models/user-session.h"

class QLabel;
class QLineEdit;
class QPushButton;

namespace client::presentation {

class LoginDialog : public QDialog {

        Q_OBJECT

        application::ClientRuntime& m_runtime;
        domain::models::UserSession m_session;

        QLineEdit* m_baseUrlEdit = nullptr;
        QLineEdit* m_loginEdit = nullptr;
        QLineEdit* m_passwordEdit = nullptr;
        QPushButton* m_loginButton = nullptr;
        QPushButton* m_registerButton = nullptr;
        QLabel* m_statusLabel = nullptr;

    public:

        explicit LoginDialog(application::ClientRuntime& runtime, QWidget* parent = nullptr);

        domain::models::UserSession session() const;

    private slots:
        void startLogin();
        void startRegister();
        void handleAuthenticationFinished(ApiResult<domain::models::UserSession> result);

    private:

        void buildUi();
        void authenticate(bool registration);
        void setBusy(bool busy);
        void showError(const ApiError& error);
};

}  // namespace client::presentation
