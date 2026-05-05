#pragma once

#include "domain/models/file-lock.h"
#include "domain/models/remote-file.h"
#include "domain/models/user-session.h"
#include "infrastructure/api/api-client.h"
#include "infrastructure/api/file-acl-api.h"
#include "infrastructure/api/file-api.h"
#include "infrastructure/api/file-content-api.h"
#include "infrastructure/api/file-lock-api.h"
#include "infrastructure/api/file-version-api.h"

#include <QHash>
#include <QMainWindow>
#include <optional>
#include <vector>

class QAction;
class QLabel;
class QPushButton;
class QTextEdit;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

namespace client::presentation {

    class MainWindow final : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(infrastructure::api::ApiClient& apiClient,
                            domain::models::UserSession session,
                            QWidget* parent = nullptr);

    protected:
        void closeEvent(QCloseEvent* event) override;

    private:
        void buildUi();
        void buildToolbar();
        void refreshFiles();
        void renderFiles(const std::vector<domain::models::RemoteFile>& files);
        void rememberLocalFile(domain::models::RemoteFile file);
        bool isLocalOnlyFile(qint64 fileId) const;
        void insertFileItem(const domain::models::RemoteFile& file);
        QTreeWidgetItem* ensureFolderItem(const QString& pathPart, QTreeWidgetItem* parent, const QString& fullPath);
        void updateSelectedFileInfo();
        void hydrateFileMeta(qint64 fileId);

        void createFile();
        void renameFile();
        void deleteFile();
        void openSelectedReadOnly();
        void openSelectedForEdit();
        void saveOpenedFile();
        void releaseOpenedLock();
        void showVersions();
        void openVersion(qint64 versionId);
        void renewLock();
        void logout();

        std::optional<qint64> selectedFileId() const;
        QString selectedLogicalName() const;
        static QString aclToText(domain::models::AclLevel aclLevel);
        static QString formatUnixSeconds(qint64 seconds);
        void setEditorState(bool enabled, bool readOnly, QString title = {});
        void showApiError(const QString& title, const infrastructure::api::ApiError& error);

    private:
        infrastructure::api::ApiClient& m_apiClient;
        domain::models::UserSession m_session;

        infrastructure::api::FileApi m_fileApi;
        infrastructure::api::FileContentApi m_contentApi;
        infrastructure::api::FileLockApi m_lockApi;
        infrastructure::api::FileAclApi m_aclApi;
        infrastructure::api::FileVersionApi m_versionApi;

        QTreeWidget* m_fileTree = nullptr;
        QTextEdit* m_editor = nullptr;
        QLabel* m_infoLabel = nullptr;
        QLabel* m_statusLabel = nullptr;
        QPushButton* m_saveButton = nullptr;
        QPushButton* m_releaseButton = nullptr;
        QTimer* m_lockRenewTimer = nullptr;

        QHash<qint64, domain::models::RemoteFile> m_filesById;
        QHash<qint64, domain::models::RemoteFile> m_localFilesById;
        QHash<qint64, QTreeWidgetItem*> m_itemsByFileId;
        QHash<QString, QTreeWidgetItem*> m_folderItemsByPath;

        std::optional<qint64> m_openedFileId;
        std::optional<qint64> m_openedLockToken;
        QString m_openedLogicalName;
    };

}
