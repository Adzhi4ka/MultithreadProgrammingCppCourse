#include "main-window.h"

#include "domain/models/acl-level.h"
#include "presentation/dialogs/group-management-dialog.h"
#include "presentation/editor-window.h"
#include "presentation/ui-format.h"
#include "presentation/widgets/file-info-widget.h"
#include "presentation/widgets/file-tree-widget.h"

#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace client::presentation {

    namespace {
        constexpr qint64 LockDurationSec = 300;
    }

    MainWindow::MainWindow(application::ClientRuntime& runtime,
                           domain::models::UserSession session,
                           QWidget* parent)
        : QMainWindow(parent),
          m_runtime(runtime),
          m_session(std::move(session)) {
        buildUi();
        connectRuntimeSignals();
        m_runtime.startNotifications();
        refreshFiles();
    }

    void MainWindow::closeEvent(QCloseEvent* event) {
        const auto editors = m_editorWindows;
        for (const auto& editor : editors) {
            if (editor) {
                editor->close();
            }
        }

        m_editorWindows.clear();
        m_runtime.stopNotifications();

        QMainWindow::closeEvent(event);
    }

    void MainWindow::buildUi() {
        setWindowTitle(QStringLiteral("File Storage Client — %1").arg(m_session.login));
        resize(1200, 720);

        buildActions();
        buildMenuBar();

        m_fileTree = new FileTreeWidget(this);
        m_fileTree->setObjectName(QStringLiteral("fileTree"));

        m_infoPanel = new FileInfoWidget(this);
        m_infoPanel->setObjectName(QStringLiteral("infoPanel"));

        auto* rightPanel = new QWidget(this);
        auto* rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->addWidget(m_infoPanel);
        rightLayout->addStretch(1);

        auto* splitter = new QSplitter(this);
        splitter->addWidget(m_fileTree);
        splitter->addWidget(rightPanel);
        splitter->setStretchFactor(0, 3);
        splitter->setStretchFactor(1, 2);
        setCentralWidget(splitter);

        m_statusLabel = new QLabel(this);
        m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
        statusBar()->addPermanentWidget(m_statusLabel, 1);

        connect(m_fileTree, &FileTreeWidget::selectedFileChanged, this, [this]() {
            m_infoPanel->setFile(m_fileTree->selectedFile());
        });

        connect(m_fileTree, &FileTreeWidget::fileActivated, this, [this](qint64) {
            openSelectedReadOnly();
        });

        connect(m_fileTree,
                &QWidget::customContextMenuRequested,
                this,
                &MainWindow::showFileContextMenu);
    }

    void MainWindow::buildActions() {
        m_showGroupsAction = new QAction(QStringLiteral("Groups / users"), this);
        m_showGroupsAction->setObjectName(QStringLiteral("showGroupsAction"));

        m_logoutAction = new QAction(QStringLiteral("Logout"), this);
        m_logoutAction->setObjectName(QStringLiteral("logoutAction"));

        connect(m_showGroupsAction, &QAction::triggered, this, &MainWindow::showGroups);
        connect(m_logoutAction, &QAction::triggered, this, &MainWindow::logout);
    }

    void MainWindow::buildMenuBar() {
        auto* accountMenu = menuBar()->addMenu(QStringLiteral("Account"));
        accountMenu->addAction(m_logoutAction);

        auto* groupsMenu = menuBar()->addMenu(QStringLiteral("Groups"));
        groupsMenu->addAction(m_showGroupsAction);
    }

    void MainWindow::connectRuntimeSignals() {
        connect(&m_runtime, &application::ClientRuntime::filesLoaded, this, &MainWindow::handleFilesLoaded);
        connect(&m_runtime, &application::ClientRuntime::fileCreated, this, &MainWindow::handleFileCreated);
        connect(&m_runtime, &application::ClientRuntime::fileRenamed, this, &MainWindow::handleFileRenamed);
        connect(&m_runtime, &application::ClientRuntime::fileDeleted, this, &MainWindow::handleFileDeleted);
        connect(&m_runtime, &application::ClientRuntime::currentDownloaded, this, &MainWindow::handleCurrentDownloaded);
        connect(&m_runtime, &application::ClientRuntime::lockAcquired, this, &MainWindow::handleLockAcquired);
        connect(&m_runtime, &application::ClientRuntime::versionsLoaded, this, &MainWindow::handleVersionsLoaded);
        connect(&m_runtime, &application::ClientRuntime::versionDownloaded, this, &MainWindow::handleVersionDownloaded);
        connect(&m_runtime, &application::ClientRuntime::notificationReceived, this, &MainWindow::handleNotification);
    }

    void MainWindow::showFileContextMenu(const QPoint& position) {
        if (auto* item = m_fileTree->itemAt(position)) {
            m_fileTree->setCurrentItem(item);
        } else {
            m_fileTree->clearSelection();
            m_fileTree->setCurrentItem(nullptr);
        }

        const auto file = selectedFile();

        QMenu menu(this);

        if (file) {
            menu.addAction(QStringLiteral("Open readonly"), this, &MainWindow::openSelectedReadOnly);

            auto* editAction = menu.addAction(QStringLiteral("Take edit"), this, &MainWindow::openSelectedForEdit);
            editAction->setEnabled(domain::models::canWrite(file->aclLevel));

            menu.addAction(QStringLiteral("Versions"), this, &MainWindow::showVersions);
            menu.addSeparator();
            menu.addAction(QStringLiteral("Rename"), this, &MainWindow::renameFile);
            menu.addAction(QStringLiteral("Delete"), this, &MainWindow::deleteFile);
            menu.addSeparator();
        }

        menu.addAction(QStringLiteral("Refresh"), this, &MainWindow::refreshFiles);
        menu.addAction(QStringLiteral("Create file"), this, &MainWindow::createFile);

        menu.exec(m_fileTree->viewport()->mapToGlobal(position));
    }

    void MainWindow::refreshFiles() {
        m_statusLabel->setText(QStringLiteral("Loading files..."));
        m_runtime.loadFilesWithMeta(m_session.userId);
    }

    void MainWindow::createFile() {
        bool ok = false;
        const auto logicalName = QInputDialog::getText(this,
                                                       QStringLiteral("Create file"),
                                                       QStringLiteral("Logical name, for example /docs/a.txt"),
                                                       QLineEdit::Normal,
                                                       QStringLiteral("/new-file.txt"),
                                                       &ok).trimmed();
        if (!ok || logicalName.isEmpty()) {
            return;
        }

        const auto maxVersions = QInputDialog::getInt(this,
                                                      QStringLiteral("Create file"),
                                                      QStringLiteral("Max version count"),
                                                      10,
                                                      1,
                                                      1000,
                                                      1,
                                                      &ok);
        if (!ok) {
            return;
        }

        m_statusLabel->setText(QStringLiteral("Creating file..."));
        m_runtime.createFile(logicalName, static_cast<quint32>(maxVersions));
    }

    void MainWindow::renameFile() {
        const auto file = selectedFile();
        if (!file) {
            return;
        }

        bool ok = false;
        const auto newName = QInputDialog::getText(this,
                                                   QStringLiteral("Rename file"),
                                                   QStringLiteral("New logical name"),
                                                   QLineEdit::Normal,
                                                   file->fullLogicalName,
                                                   &ok).trimmed();
        if (!ok || newName.isEmpty()) {
            return;
        }

        m_statusLabel->setText(QStringLiteral("Renaming file..."));
        m_runtime.renameFile(file->id, newName);
    }

    void MainWindow::deleteFile() {
        const auto file = selectedFile();
        if (!file) {
            return;
        }

        if (QMessageBox::question(this,
                                  QStringLiteral("Delete file"),
                                  QStringLiteral("Delete %1?").arg(file->fullLogicalName)) != QMessageBox::Yes) {
            return;
        }

        m_statusLabel->setText(QStringLiteral("Deleting file..."));
        m_runtime.deleteFile(file->id);
    }

    void MainWindow::openSelectedReadOnly() {
        const auto file = selectedFile();
        if (!file) {
            return;
        }

        m_pendingReadOnlyFile = file;
        m_statusLabel->setText(QStringLiteral("Downloading file..."));
        m_runtime.downloadCurrent(file->id);
    }

    void MainWindow::openSelectedForEdit() {
        const auto file = selectedFile();
        if (!file) {
            return;
        }

        m_pendingEditFile = file;
        m_pendingEditLock.reset();

        m_statusLabel->setText(QStringLiteral("Acquiring lock..."));
        m_runtime.acquireLock(file->id, LockDurationSec);
    }

    void MainWindow::showVersions() {
        const auto file = selectedFile();
        if (!file) {
            return;
        }

        m_pendingVersionsFileId = file->id;

        m_statusLabel->setText(QStringLiteral("Loading versions..."));
        m_runtime.loadVersions(file->id);
    }

    void MainWindow::openVersion(qint64 versionId) {
        m_pendingVersionId = versionId;

        m_statusLabel->setText(QStringLiteral("Downloading version..."));
        m_runtime.downloadVersion(versionId);
    }

    void MainWindow::showGroups() {
        GroupManagementDialog dialog(m_runtime, m_session, this);
        dialog.exec();
    }

    void MainWindow::logout() {
        const auto editors = m_editorWindows;
        for (const auto& editor : editors) {
            if (editor) {
                editor->close();
            }
        }

        m_editorWindows.clear();

        m_runtime.logout();
        close();
    }

    void MainWindow::handleFilesLoaded(qint64 currentUserId, ApiResult<std::vector<domain::models::RemoteFile>> result) {
        if (currentUserId != m_session.userId) {
            return;
        }

        if (!result) {
            showApiError(QStringLiteral("Failed to load files"), result.error());
            m_statusLabel->clear();
            return;
        }

        m_fileTree->setFiles(*result);
        m_statusLabel->setText(QStringLiteral("Loaded %1 files").arg(m_fileTree->fileCount()));
    }

    void MainWindow::handleFileCreated(ApiResult<domain::models::RemoteFile> result) {
        if (!result) {
            showApiError(QStringLiteral("Failed to create file"), result.error());
            m_statusLabel->clear();
            return;
        }

        refreshFiles();
    }

    void MainWindow::handleFileRenamed(qint64, ApiResult<domain::models::RemoteFile> result) {
        if (!result) {
            showApiError(QStringLiteral("Failed to rename file"), result.error());
            m_statusLabel->clear();
            return;
        }

        refreshFiles();
    }

    void MainWindow::handleFileDeleted(qint64, ApiResult<void> result) {
        if (!result) {
            showApiError(QStringLiteral("Failed to delete file"), result.error());
            m_statusLabel->clear();
            return;
        }

        refreshFiles();
    }

    void MainWindow::handleCurrentDownloaded(qint64 fileId, ApiResult<QByteArray> result) {
        if (m_pendingEditFile && m_pendingEditFile->id == fileId && m_pendingEditLock) {
            m_statusLabel->clear();

            const auto file = *m_pendingEditFile;
            const auto lock = *m_pendingEditLock;

            m_pendingEditFile.reset();
            m_pendingEditLock.reset();

            if (!result) {
                showApiError(QStringLiteral("Failed to download file"), result.error());
                m_runtime.releaseLock(lock.fileId, lock.lockToken);
                return;
            }

            auto* editor = EditorWindow::createEditable(m_runtime,
                                                        file.id,
                                                        lock.lockToken,
                                                        file.fullLogicalName,
                                                        std::move(*result));
            registerEditor(editor);
            editor->show();

            refreshFiles();
            return;
        }

        if (!m_pendingReadOnlyFile || m_pendingReadOnlyFile->id != fileId) {
            return;
        }

        m_statusLabel->clear();

        const auto file = *m_pendingReadOnlyFile;
        m_pendingReadOnlyFile.reset();

        if (!result) {
            showApiError(QStringLiteral("Failed to download file"), result.error());
            return;
        }

        auto* editor = EditorWindow::createReadOnly(m_runtime, file.fullLogicalName, std::move(*result));
        registerEditor(editor);
        editor->show();
    }

    void MainWindow::handleLockAcquired(qint64 fileId, ApiResult<domain::models::FileLock> result) {
        if (!m_pendingEditFile || m_pendingEditFile->id != fileId) {
            return;
        }

        if (!result) {
            m_pendingEditFile.reset();
            m_pendingEditLock.reset();

            m_statusLabel->clear();
            showApiError(QStringLiteral("Failed to acquire file lock"), result.error());
            return;
        }

        m_pendingEditLock = *result;

        m_statusLabel->setText(QStringLiteral("Downloading file..."));
        m_runtime.downloadCurrent(fileId);
    }

    void MainWindow::handleVersionsLoaded(qint64 fileId, ApiResult<std::vector<domain::models::FileVersion>> result) {
        if (!m_pendingVersionsFileId || *m_pendingVersionsFileId != fileId) {
            return;
        }

        m_pendingVersionsFileId.reset();
        m_statusLabel->clear();

        if (!result) {
            showApiError(QStringLiteral("Failed to load versions"), result.error());
            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("File versions"));
        dialog.resize(520, 360);

        auto* list = new QListWidget(&dialog);
        for (const auto& version : *result) {
            auto* item = new QListWidgetItem(QStringLiteral("v%1 | id=%2 | %3")
                                                 .arg(version.version)
                                                 .arg(version.id)
                                                 .arg(formatUnixSeconds(version.createdAt)),
                                             list);
            item->setData(Qt::UserRole, version.id);
        }

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Close, &dialog);

        auto* layout = new QVBoxLayout(&dialog);
        layout->addWidget(list);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
            if (auto* item = list->currentItem()) {
                openVersion(item->data(Qt::UserRole).toLongLong());
                dialog.accept();
            }
        });

        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        dialog.exec();
    }

    void MainWindow::handleVersionDownloaded(qint64 versionId, ApiResult<QByteArray> result) {
        if (!m_pendingVersionId || *m_pendingVersionId != versionId) {
            return;
        }

        m_pendingVersionId.reset();
        m_statusLabel->clear();

        if (!result) {
            showApiError(QStringLiteral("Failed to download version"), result.error());
            return;
        }

        auto* editor = EditorWindow::createReadOnly(m_runtime,
                                                    QStringLiteral("version %1").arg(versionId),
                                                    std::move(*result));
        registerEditor(editor);
        editor->show();
    }

    void MainWindow::handleNotification(ApiResult<domain::models::NotificationEvent> result) {
        if (!result) {
            m_statusLabel->setText(QStringLiteral("Notification stream disconnected"));
            return;
        }

        const auto& event = *result;
        if (event.name == QStringLiteral("connected")) {
            m_statusLabel->setText(QStringLiteral("Notifications connected"));
            return;
        }

        if (event.name == QStringLiteral("file_created")
            || event.name == QStringLiteral("file_locked")
            || event.name == QStringLiteral("file_unlocked")
            || event.name == QStringLiteral("group_assigned")) {
            refreshFiles();
        }
    }

    std::optional<domain::models::RemoteFile> MainWindow::selectedFile() const {
        return m_fileTree->selectedFile();
    }

    void MainWindow::registerEditor(EditorWindow* window) {
        m_editorWindows.push_back(QPointer<EditorWindow>{window});

        connect(window, &QObject::destroyed, this, [this, window]() {
            m_editorWindows.erase(std::remove_if(m_editorWindows.begin(),
                                                 m_editorWindows.end(),
                                                 [window](const QPointer<EditorWindow>& current) {
                                                     return current.isNull() || current.data() == window;
                                                 }),
                                  m_editorWindows.end());

            refreshFiles();
        });
    }

    void MainWindow::showApiError(const QString& title, const ApiError& error) {
        const auto message = QStringLiteral("%1%2")
            .arg(error.httpStatus > 0
                     ? QStringLiteral("HTTP %1: ").arg(error.httpStatus)
                     : QStringLiteral("Network error: "))
            .arg(error.message.isEmpty() ? QStringLiteral("unknown error") : error.message);

        QMessageBox::warning(this, title, message);
    }

}