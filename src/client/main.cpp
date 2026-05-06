#include "application/client-runtime.h"
#include "infrastructure/repositories/session-repository.h"
#include "presentation/login-dialog.h"
#include "presentation/main-window.h"
#include "presentation/styles/app-style.h"

#include <QApplication>
#include <QColor>
#include <QDialog>
#include <QPalette>
#include <QStyleFactory>
#include <QUrl>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("File Storage Client");
    client::presentation::styles::applyDarkTheme(app);

    client::application::ClientRuntime runtime{QUrl{"http://127.0.0.1:8080"}};
    client::infrastructure::repositories::SessionRepository sessionRepository;

    client::presentation::LoginDialog loginDialog{runtime};
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    sessionRepository.save(loginDialog.session());

    runtime.setSession(*sessionRepository.current());

    client::presentation::MainWindow mainWindow{runtime, *sessionRepository.current()};
    mainWindow.show();

    return app.exec();
}
