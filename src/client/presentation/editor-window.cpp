#include "editor-window.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>

#include <utility>

namespace client::presentation {

    EditorWindow* EditorWindow::createReadOnly(application::ClientRuntime& runtime,
                                               QString title,
                                               QByteArray content,
                                               QWidget* parent) {
        auto* window = new EditorWindow(runtime, std::move(title), std::move(content), std::nullopt, std::nullopt, parent);
        window->setAttribute(Qt::WA_DeleteOnClose);
        return window;
    }

    EditorWindow* EditorWindow::createEditable(application::ClientRuntime& runtime,
                                               qint64 fileId,
                                               qint64 lockToken,
                                               QString logicalName,
                                               QByteArray content,
                                               QWidget* parent) {
        auto* window = new EditorWindow(runtime, std::move(logicalName), std::move(content), fileId, lockToken, parent);
        window->setAttribute(Qt::WA_DeleteOnClose);
        return window;
    }

    EditorWindow::EditorWindow(application::ClientRuntime& runtime,
                               QString title,
                               QByteArray content,
                               std::optional<qint64> fileId,
                               std::optional<qint64> lockToken,
                               QWidget* parent)
        : QMainWindow(parent),
          m_runtime(runtime),
          m_title(std::move(title)),
          m_fileId(fileId),
          m_lockToken(lockToken) {
        buildUi(content);
        connectRuntimeSignals();
    }

    void EditorWindow::buildUi(const QByteArray& content) {
        setWindowTitle(m_lockToken ? QStringLiteral("Editing — %1").arg(m_title)
                                   : QStringLiteral("Readonly — %1").arg(m_title));
        resize(860, 640);

        auto* toolbar = addToolBar(QStringLiteral("Editor"));
        toolbar->setMovable(false);

        m_saveButton = new QPushButton(QStringLiteral("Save"), this);
        m_saveButton->setObjectName(QStringLiteral("saveButton"));
        m_releaseButton = new QPushButton(QStringLiteral("Release lock"), this);
        m_releaseButton->setObjectName(QStringLiteral("releaseButton"));
        toolbar->addWidget(m_saveButton);
        toolbar->addWidget(m_releaseButton);

        m_editor = new QTextEdit(this);
        m_editor->setObjectName(QStringLiteral("editor"));
        m_editor->setPlainText(QString::fromUtf8(content));
        m_editor->setReadOnly(!m_lockToken.has_value());
        setCentralWidget(m_editor);

        m_statusLabel = new QLabel(this);
        m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
        statusBar()->addPermanentWidget(m_statusLabel, 1);

        m_saveButton->setEnabled(m_lockToken.has_value());
        m_releaseButton->setEnabled(m_lockToken.has_value());

        m_renewTimer = new QTimer(this);
        m_renewTimer->setInterval(120'000);

        connect(m_saveButton, &QPushButton::clicked, this, &EditorWindow::save);
        connect(m_releaseButton, &QPushButton::clicked, this, &EditorWindow::releaseLock);
        connect(m_renewTimer, &QTimer::timeout, this, &EditorWindow::renewLock);

        if (m_lockToken) {
            m_renewTimer->start();
        }
    }

    void EditorWindow::connectRuntimeSignals() {
        connect(&m_runtime, &application::ClientRuntime::currentUploaded, this, &EditorWindow::handleCurrentUploaded);
        connect(&m_runtime, &application::ClientRuntime::lockReleased, this, &EditorWindow::handleLockReleased);
        connect(&m_runtime, &application::ClientRuntime::lockRenewed, this, &EditorWindow::handleLockRenewed);
    }

    void EditorWindow::closeEvent(QCloseEvent* event) {
        if (m_fileId && m_lockToken && !m_releaseStarted) {
            const auto fileId = *m_fileId;
            const auto lockToken = *m_lockToken;
            m_lockToken.reset();
            m_renewTimer->stop();
            m_runtime.releaseLock(fileId, lockToken);
        }

        QMainWindow::closeEvent(event);
    }

    void EditorWindow::save() {
        if (!m_fileId || !m_lockToken) {
            return;
        }

        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Saving..."));
        m_runtime.uploadCurrent(*m_fileId, m_editor->toPlainText().toUtf8());
    }

    void EditorWindow::releaseLock() {
        if (!m_fileId || !m_lockToken || m_releaseStarted) {
            return;
        }

        m_releaseStarted = true;
        m_renewTimer->stop();
        setBusy(true);
        m_statusLabel->setText(QStringLiteral("Releasing lock..."));
        m_runtime.releaseLock(*m_fileId, *m_lockToken);
    }

    void EditorWindow::renewLock() {
        if (!m_fileId || !m_lockToken) {
            return;
        }

        m_runtime.renewLock(*m_fileId, *m_lockToken, LockDurationSec);
    }

    void EditorWindow::handleCurrentUploaded(qint64 fileId, ApiResult<domain::models::FileVersion> result) {
        if (!m_fileId || *m_fileId != fileId) {
            return;
        }

        setBusy(false);

        if (!result) {
            showApiError(QStringLiteral("Failed to save file"), result.error());
            m_statusLabel->clear();
            return;
        }

        m_statusLabel->setText(QStringLiteral("Saved version %1").arg(result->version));
    }

    void EditorWindow::handleLockReleased(qint64 fileId, qint64 lockToken, ApiResult<void> result) {
        if (!m_fileId || *m_fileId != fileId) {
            return;
        }
        if (m_lockToken && *m_lockToken != lockToken) {
            return;
        }

        setBusy(false);
        m_releaseStarted = false;

        if (!result) {
            showApiError(QStringLiteral("Failed to release lock"), result.error());
            return;
        }

        m_lockToken.reset();
        m_editor->setReadOnly(true);
        m_saveButton->setEnabled(false);
        m_releaseButton->setEnabled(false);
        m_statusLabel->setText(QStringLiteral("Lock released. Window is readonly now."));
        setWindowTitle(QStringLiteral("Readonly — %1").arg(m_title));
    }

    void EditorWindow::handleLockRenewed(qint64 fileId, qint64 lockToken, ApiResult<void> result) {
        if (!m_fileId || !m_lockToken || *m_fileId != fileId || *m_lockToken != lockToken) {
            return;
        }

        if (!result) {
            m_renewTimer->stop();
            m_statusLabel->setText(QStringLiteral("Failed to renew lock"));
        }
    }

    void EditorWindow::setBusy(bool busy) {
        m_editor->setEnabled(!busy);
        m_saveButton->setEnabled(!busy && m_lockToken.has_value());
        m_releaseButton->setEnabled(!busy && m_lockToken.has_value());
    }

    void EditorWindow::showApiError(const QString& title, const ApiError& error) {
        const auto message = QStringLiteral("%1%2")
            .arg(error.httpStatus > 0 ? QStringLiteral("HTTP %1: ").arg(error.httpStatus) : QStringLiteral("Network error: "))
            .arg(error.message.isEmpty() ? QStringLiteral("unknown error") : error.message);
        QMessageBox::warning(this, title, message);
    }

}
