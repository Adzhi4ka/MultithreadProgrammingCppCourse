#include "infrastructure/api/api-client.h"
#include "infrastructure/api/auth-api.h"
#include "infrastructure/repositories/in-memory-session-repository.h"
#include "presentation/login-dialog.h"
#include "presentation/main-window.h"

#include <QApplication>
#include <QColor>
#include <QDialog>
#include <QPalette>
#include <QStyleFactory>
#include <QUrl>

namespace {

    void applyLightTheme(QApplication& app) {
        app.setStyle(QStyleFactory::create("Fusion"));

        QPalette palette;
        palette.setColor(QPalette::Window, QColor("#f6f7fb"));
        palette.setColor(QPalette::WindowText, QColor("#1f2937"));
        palette.setColor(QPalette::Base, QColor("#ffffff"));
        palette.setColor(QPalette::AlternateBase, QColor("#f8fafc"));
        palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipText, QColor("#1f2937"));
        palette.setColor(QPalette::Text, QColor("#111827"));
        palette.setColor(QPalette::Button, QColor("#ffffff"));
        palette.setColor(QPalette::ButtonText, QColor("#111827"));
        palette.setColor(QPalette::BrightText, QColor("#ef4444"));
        palette.setColor(QPalette::Highlight, QColor("#dbeafe"));
        palette.setColor(QPalette::HighlightedText, QColor("#111827"));
        palette.setColor(QPalette::PlaceholderText, QColor("#6b7280"));
        palette.setColor(QPalette::Link, QColor("#2563eb"));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#6b7280"));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#6b7280"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#6b7280"));
        app.setPalette(palette);

        app.setStyleSheet(R"(
            QWidget {
                color: #1f2937;
                background-color: #f6f7fb;
                selection-background-color: #dbeafe;
                selection-color: #111827;
            }

            QMainWindow, QDialog, QMessageBox, QInputDialog {
                background-color: #f6f7fb;
                color: #1f2937;
            }

            QLabel, QCheckBox, QRadioButton, QGroupBox, QStatusBar {
                color: #1f2937;
                background-color: transparent;
            }

            QTreeWidget, QTextEdit, QPlainTextEdit, QLineEdit, QListWidget, QTableWidget, QTableView, QListView {
                color: #111827;
                background-color: #ffffff;
                alternate-background-color: #f8fafc;
                border: 1px solid #d8dce5;
                border-radius: 6px;
                padding: 4px;
            }

            QTextEdit:disabled, QPlainTextEdit:disabled, QLineEdit:disabled {
                color: #374151;
                background-color: #f3f4f6;
            }

            QTreeWidget::item, QListWidget::item {
                color: #111827;
                background-color: transparent;
                padding: 3px;
            }

            QTreeWidget::item:selected, QListWidget::item:selected {
                color: #111827;
                background-color: #dbeafe;
            }

            QTreeWidget::item:hover, QListWidget::item:hover {
                background-color: #eef3ff;
            }

            QHeaderView::section {
                color: #111827;
                background-color: #eef0f4;
                border: 0;
                border-right: 1px solid #d8dce5;
                border-bottom: 1px solid #d8dce5;
                padding: 5px 8px;
                font-weight: 600;
            }

            QPushButton, QToolButton {
                color: #111827;
                background-color: #ffffff;
                padding: 6px 12px;
                border: 1px solid #c9cfdd;
                border-radius: 6px;
            }

            QPushButton:hover, QToolButton:hover {
                background-color: #eef3ff;
            }

            QPushButton:pressed, QToolButton:pressed {
                background-color: #dbeafe;
            }

            QPushButton:disabled, QToolButton:disabled {
                color: #6b7280;
                background-color: #eef0f4;
            }

            QToolBar {
                color: #111827;
                background-color: #ffffff;
                border-bottom: 1px solid #d8dce5;
                spacing: 4px;
                padding: 4px;
            }

            QMenuBar, QMenu {
                color: #111827;
                background-color: #ffffff;
            }

            QMenu::item:selected {
                color: #111827;
                background-color: #dbeafe;
            }

            QSplitter::handle {
                background-color: #d8dce5;
            }
        )");
    }

}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("File Storage Client");
    applyLightTheme(app);

    client::infrastructure::api::ApiClient apiClient{QUrl{"http://127.0.0.1:8080"}};
    client::infrastructure::api::AuthApi authApi{apiClient};
    client::infrastructure::repositories::InMemorySessionRepository sessionRepository;

    client::presentation::LoginDialog loginDialog{apiClient, authApi};
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    sessionRepository.save(loginDialog.session());

    client::presentation::MainWindow mainWindow{apiClient, *sessionRepository.current()};
    mainWindow.show();

    return app.exec();
}
