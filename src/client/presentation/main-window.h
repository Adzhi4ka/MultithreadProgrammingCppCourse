#pragma once

#include <QByteArray>
#include <QList>
#include <QMainWindow>
#include <QPointer>
#include <optional>
#include <vector>

#include "application/client-runtime.h"
#include "domain/models/file-lock.h"
#include "domain/models/file-version.h"
#include "domain/models/notification-event.h"
#include "domain/models/remote-file.h"
#include "domain/models/user-session.h"

class QAction;
class QCloseEvent;
class QLabel;
class QPoint;

namespace client::presentation {

class EditorWindow;
class FileInfoWidget;
class FileTreeWidget;

class MainWindow : public QMainWindow {

        Q_OBJECT

        application::ClientRuntime& m_runtime;
        domain::models::UserSession m_session;

        QAction* m_showGroupsAction = nullptr;
        QAction* m_logoutAction = nullptr;

        FileTreeWidget* m_fileTree = nullptr;
        FileInfoWidget* m_infoPanel = nullptr;
        QLabel* m_statusLabel = nullptr;

        QList<QPointer<EditorWindow>> m_editorWindows;
        std::optional<domain::models::RemoteFile> m_pendingReadOnlyFile;
        std::optional<domain::models::RemoteFile> m_pendingEditFile;
        std::optional<domain::models::FileLock> m_pendingEditLock;
        std::optional<qint64> m_pendingVersionsFileId;
        std::optional<qint64> m_pendingVersionId;

    public:

        explicit MainWindow(application::ClientRuntime& runtime, domain::models::UserSession session,
                            QWidget* parent = nullptr);

    private slots:

        void refreshFiles();
        void createFile();
        void renameFile();
        void deleteFile();
        void openSelectedReadOnly();
        void openSelectedForEdit();
        void showVersions();
        void openVersion(qint64 versionId);
        void showGroups();
        void logout();

        void handleFilesLoaded(qint64 currentUserId, ApiResult<std::vector<domain::models::RemoteFile>> result);
        void handleFileCreated(ApiResult<domain::models::RemoteFile> result);
        void handleFileRenamed(qint64 fileId, ApiResult<domain::models::RemoteFile> result);
        void handleFileDeleted(qint64 fileId, ApiResult<void> result);
        void handleCurrentDownloaded(qint64 fileId, ApiResult<QByteArray> result);
        void handleLockAcquired(qint64 fileId, ApiResult<domain::models::FileLock> result);
        void handleVersionsLoaded(qint64 fileId, ApiResult<std::vector<domain::models::FileVersion>> result);
        void handleVersionDownloaded(qint64 versionId, ApiResult<QByteArray> result);
        void handleNotification(ApiResult<domain::models::NotificationEvent> result);

    private:

        void closeEvent(QCloseEvent* event) override;

        void buildUi();
        void buildActions();
        void buildMenuBar();
        void connectRuntimeSignals();
        void showFileContextMenu(const QPoint& position);

        std::optional<domain::models::RemoteFile> selectedFile() const;
        void registerEditor(EditorWindow* window);
        void showApiError(const QString& title, const ApiError& error);
};

}  // namespace client::presentation