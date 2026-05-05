#include "main-window.h"

#include <QAbstractItemView>
#include <QAction>
#include <QCloseEvent>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace client::presentation {

    namespace {
        constexpr qint64 LockDurationSec = 300;
        constexpr int FileIdRole = Qt::UserRole + 1;
    }

    MainWindow::MainWindow(infrastructure::api::ApiClient& apiClient,
                           domain::models::UserSession session,
                           QWidget* parent)
        : QMainWindow(parent),
          m_apiClient(apiClient),
          m_session(std::move(session)),
          m_fileApi(apiClient, this),
          m_contentApi(apiClient, this),
          m_lockApi(apiClient, this),
          m_aclApi(apiClient, this),
          m_versionApi(apiClient, this) {
        buildUi();
        refreshFiles();
    }

    void MainWindow::closeEvent(QCloseEvent* event) {
        if (m_openedFileId && m_openedLockToken) {
            const auto fileId = *m_openedFileId;
            const auto lockToken = *m_openedLockToken;
            m_openedFileId.reset();
            m_openedLockToken.reset();
            m_lockRenewTimer->stop();
            m_lockApi.release(fileId, lockToken, [](infrastructure::api::ApiResult<void>) {});
        }

        QMainWindow::closeEvent(event);
    }

    void MainWindow::buildUi() {
        setWindowTitle(QStringLiteral("File Storage Client — %1").arg(m_session.login));
        resize(1200, 720);

        buildToolbar();

        m_fileTree = new QTreeWidget(this);
        m_fileTree->setHeaderLabels({"Name", "Access", "Lock", "Created by", "Created at"});
        m_fileTree->setAlternatingRowColors(true);
        m_fileTree->setSelectionMode(QAbstractItemView::SingleSelection);
        m_fileTree->header()->setStretchLastSection(false);
        m_fileTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_fileTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        m_fileTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        m_fileTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        m_fileTree->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

        m_infoLabel = new QLabel("Select file", this);
        m_infoLabel->setWordWrap(true);
        m_infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        m_editor = new QTextEdit(this);
        m_editor->setPlaceholderText("Open a text file on read or edit");
        m_editor->setReadOnly(true);
        m_editor->setEnabled(false);

        m_saveButton = new QPushButton("Save", this);
        m_releaseButton = new QPushButton("Release lock", this);
        m_saveButton->setEnabled(false);
        m_releaseButton->setEnabled(false);

        auto* editorButtons = new QHBoxLayout;
        editorButtons->addWidget(m_saveButton);
        editorButtons->addWidget(m_releaseButton);
        editorButtons->addStretch();

        auto* rightPanel = new QWidget(this);
        auto* rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->addWidget(m_infoLabel);
        rightLayout->addWidget(m_editor, 1);
        rightLayout->addLayout(editorButtons);

        auto* splitter = new QSplitter(this);
        splitter->addWidget(m_fileTree);
        splitter->addWidget(rightPanel);
        splitter->setStretchFactor(0, 2);
        splitter->setStretchFactor(1, 3);
        setCentralWidget(splitter);

        m_statusLabel = new QLabel(this);
        statusBar()->addPermanentWidget(m_statusLabel, 1);

        m_lockRenewTimer = new QTimer(this);
        m_lockRenewTimer->setInterval(120'000);

        connect(m_fileTree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::updateSelectedFileInfo);
        connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::saveOpenedFile);
        connect(m_releaseButton, &QPushButton::clicked, this, &MainWindow::releaseOpenedLock);
        connect(m_lockRenewTimer, &QTimer::timeout, this, &MainWindow::renewLock);
    }

    void MainWindow::buildToolbar() {
        auto* toolbar = addToolBar("Main");
        toolbar->setMovable(false);

        toolbar->addAction("Refresh", this, &MainWindow::refreshFiles);
        toolbar->addSeparator();
        toolbar->addAction("Create", this, &MainWindow::createFile);
        toolbar->addAction("Rename", this, &MainWindow::renameFile);
        toolbar->addAction("Delete", this, &MainWindow::deleteFile);
        toolbar->addSeparator();
        toolbar->addAction("Open readonly", this, &MainWindow::openSelectedReadOnly);
        toolbar->addAction("Take edit", this, &MainWindow::openSelectedForEdit);
        toolbar->addAction("Versions", this, &MainWindow::showVersions);
        toolbar->addSeparator();
        toolbar->addAction("Logout", this, &MainWindow::logout);
    }

    void MainWindow::refreshFiles() {
        m_statusLabel->setText("Loading files...");
        m_fileApi.getAll([this](infrastructure::api::FileApi::FilesResult result) mutable {
            if (!result) {
                showApiError("Failed to load files", result.error());
                m_statusLabel->clear();
                return;
            }

            renderFiles(*result);
            m_statusLabel->setText(QStringLiteral("Loaded %1 files").arg(static_cast<qlonglong>(m_filesById.size())));
        });
    }

    void MainWindow::renderFiles(const std::vector<domain::models::RemoteFile>& files) {
        std::vector<domain::models::RemoteFile> visibleFiles = files;
        QHash<qint64, std::size_t> indexByFileId;

        for (std::size_t i = 0; i < visibleFiles.size(); ++i) {
            indexByFileId.insert(visibleFiles[i].id, i);
        }

        // MVP workaround: the current server filters GET /api/files by ACL.
        // A just-created file may exist on the server but be absent from GET /api/files,
        // so keep files created/renamed in this client session visible locally.
        for (auto it = m_localFilesById.cbegin(); it != m_localFilesById.cend(); ++it) {
            if (auto indexIt = indexByFileId.find(it.key()); indexIt != indexByFileId.end()) {
                visibleFiles[*indexIt] = it.value();
            } else {
                indexByFileId.insert(it.key(), visibleFiles.size());
                visibleFiles.emplace_back(it.value());
            }
        }

        std::sort(visibleFiles.begin(), visibleFiles.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.fullLogicalName.localeAwareCompare(rhs.fullLogicalName) < 0;
        });

        m_fileTree->clear();
        m_filesById.clear();
        m_itemsByFileId.clear();
        m_folderItemsByPath.clear();

        for (const auto& file : visibleFiles) {
            m_filesById.insert(file.id, file);
            insertFileItem(file);
        }

        m_fileTree->expandToDepth(1);

        for (const auto& file : visibleFiles) {
            hydrateFileMeta(file.id);
        }
    }

    void MainWindow::rememberLocalFile(domain::models::RemoteFile file) {
        if (file.createdBy == m_session.userId && file.aclLevel == domain::models::AclLevel::NoProperty) {
            file.aclLevel = domain::models::AclLevel::ReadWrite;
        }

        m_localFilesById.insert(file.id, std::move(file));
    }

    bool MainWindow::isLocalOnlyFile(qint64 fileId) const {
        return m_localFilesById.contains(fileId);
    }

    void MainWindow::insertFileItem(const domain::models::RemoteFile& file) {
        auto logicalName = file.fullLogicalName;
        logicalName.remove(QRegularExpression(QStringLiteral("^/+")));

        const auto parts = logicalName.split('/', Qt::SkipEmptyParts);
        if (parts.isEmpty()) {
            return;
        }

        QTreeWidgetItem* parent = nullptr;
        QString currentPath;

        for (int i = 0; i < parts.size() - 1; ++i) {
            if (!currentPath.isEmpty()) {
                currentPath += '/';
            }
            currentPath += parts[i];
            parent = ensureFolderItem(parts[i], parent, currentPath);
        }

        auto* item = parent
            ? new QTreeWidgetItem(parent)
            : new QTreeWidgetItem(m_fileTree);

        item->setText(0, parts.constLast());
        item->setText(1, "visible");
        item->setText(2, "checking...");
        item->setText(3, QString::number(file.createdBy));
        item->setText(4, formatUnixSeconds(file.createdAt));
        item->setData(0, FileIdRole, file.id);

        m_itemsByFileId.insert(file.id, item);
    }

    QTreeWidgetItem* MainWindow::ensureFolderItem(const QString& pathPart, QTreeWidgetItem* parent, const QString& fullPath) {
        if (auto it = m_folderItemsByPath.find(fullPath); it != m_folderItemsByPath.end()) {
            return it.value();
        }

        auto* item = parent
            ? new QTreeWidgetItem(parent)
            : new QTreeWidgetItem(m_fileTree);

        item->setText(0, pathPart);
        item->setFirstColumnSpanned(false);
        m_folderItemsByPath.insert(fullPath, item);
        return item;
    }

    void MainWindow::updateSelectedFileInfo() {
        const auto fileId = selectedFileId();
        if (!fileId) {
            m_infoLabel->setText("Select file");
            return;
        }

        const auto file = m_filesById.value(*fileId);
        m_infoLabel->setText(QStringLiteral(
            "<b>%1</b><br>"
            "ID: %2<br>"
            "Current version ID: %3<br>"
            "Max versions: %4<br>"
            "Created by: %5<br>"
            "Created at: %6<br>"
            "Access: %7<br>"
            "Lock: %8")
            .arg(file.fullLogicalName.toHtmlEscaped())
            .arg(file.id)
            .arg(file.currentVersionId)
            .arg(file.maxVersionCount)
            .arg(file.createdBy)
            .arg(formatUnixSeconds(file.createdAt))
            .arg(aclToText(file.aclLevel))
            .arg(file.hasActiveLock
                 ? QStringLiteral("locked by %1 until %2").arg(file.lockedByUserId.value_or(0)).arg(formatUnixSeconds(file.lockLeaseUntil.value_or(0)))
                 : QStringLiteral("free")));
    }

    void MainWindow::hydrateFileMeta(qint64 fileId) {
        m_aclApi.getUserAcl(fileId, m_session.userId, [this, fileId](infrastructure::api::FileAclApi::UserAclResult result) mutable {
            auto itemIt = m_itemsByFileId.find(fileId);
            auto fileIt = m_filesById.find(fileId);
            if (itemIt == m_itemsByFileId.end() || fileIt == m_filesById.end()) {
                return;
            }

            if (result) {
                auto aclLevel = result->aclLevel;
                if (aclLevel == domain::models::AclLevel::NoProperty
                    && isLocalOnlyFile(fileId)
                    && fileIt.value().createdBy == m_session.userId) {
                    aclLevel = domain::models::AclLevel::ReadWrite;
                }

                fileIt.value().aclLevel = aclLevel;
                itemIt.value()->setText(1, aclToText(aclLevel));
                updateSelectedFileInfo();
            }
        });

        m_lockApi.getActive(fileId, [this, fileId](infrastructure::api::FileLockApi::LockResult result) mutable {
            auto itemIt = m_itemsByFileId.find(fileId);
            auto fileIt = m_filesById.find(fileId);
            if (itemIt == m_itemsByFileId.end() || fileIt == m_filesById.end()) {
                return;
            }

            if (result) {
                fileIt.value().hasActiveLock = true;
                fileIt.value().lockedByUserId = result->userId;
                fileIt.value().lockLeaseUntil = result->leaseUntil;
                itemIt.value()->setText(2, QStringLiteral("locked by %1").arg(result->userId));
            } else {
                fileIt.value().hasActiveLock = false;
                fileIt.value().lockedByUserId.reset();
                fileIt.value().lockLeaseUntil.reset();
                itemIt.value()->setText(2, "free");
            }

            updateSelectedFileInfo();
        });
    }

    void MainWindow::createFile() {
        bool ok = false;
        const auto logicalName = QInputDialog::getText(this,
                                                       "Create file",
                                                       "Logical name, for example /docs/a.txt",
                                                       QLineEdit::Normal,
                                                       "/new-file.txt",
                                                       &ok).trimmed();
        if (!ok || logicalName.isEmpty()) {
            return;
        }

        const auto maxVersions = QInputDialog::getInt(this, "Create file", "Max version count", 10, 1, 1000, 1, &ok);
        if (!ok) {
            return;
        }

        m_fileApi.create(logicalName, static_cast<quint32>(maxVersions), [this](infrastructure::api::FileApi::FileResult result) mutable {
            if (!result) {
                showApiError("Failed to create file", result.error());
                return;
            }

            rememberLocalFile(*result);
            refreshFiles();
        });
    }

    void MainWindow::renameFile() {
        const auto fileId = selectedFileId();
        if (!fileId) {
            return;
        }

        bool ok = false;
        const auto newName = QInputDialog::getText(this,
                                                   "Rename file",
                                                   "New logical name",
                                                   QLineEdit::Normal,
                                                   selectedLogicalName(),
                                                   &ok).trimmed();
        if (!ok || newName.isEmpty()) {
            return;
        }

        m_fileApi.rename(*fileId, newName, [this](infrastructure::api::FileApi::FileResult result) mutable {
            if (!result) {
                showApiError("Failed to rename file", result.error());
                return;
            }

            rememberLocalFile(*result);
            refreshFiles();
        });
    }

    void MainWindow::deleteFile() {
        const auto fileId = selectedFileId();
        if (!fileId) {
            return;
        }

        if (QMessageBox::question(this, "Delete file", "Delete selected file?") != QMessageBox::Yes) {
            return;
        }

        const auto removedFileId = *fileId;
        m_fileApi.remove(removedFileId, [this, removedFileId](infrastructure::api::ApiResult<void> result) mutable {
            if (!result) {
                showApiError("Failed to delete file", result.error());
                return;
            }

            m_localFilesById.remove(removedFileId);

            if (m_openedFileId == selectedFileId()) {
                m_editor->clear();
                m_openedFileId.reset();
                m_openedLockToken.reset();
                setEditorState(false, true);
            }
            refreshFiles();
        });
    }

    void MainWindow::openSelectedReadOnly() {
        const auto fileId = selectedFileId();
        if (!fileId) {
            return;
        }

        m_statusLabel->setText("Downloading file...");
        m_contentApi.downloadCurrent(*fileId, [this, fileId](infrastructure::api::FileContentApi::BytesResult result) mutable {
            m_statusLabel->clear();
            if (!result) {
                showApiError("Failed to download file", result.error());
                return;
            }

            m_openedFileId = fileId;
            m_openedLogicalName = selectedLogicalName();
            m_editor->setPlainText(QString::fromUtf8(*result));
            setEditorState(true, true, QStringLiteral("Readonly: %1").arg(m_openedLogicalName));
        });
    }

    void MainWindow::openSelectedForEdit() {
        const auto fileId = selectedFileId();
        if (!fileId) {
            return;
        }

        m_statusLabel->setText("Acquiring lock...");
        m_lockApi.acquire(*fileId, LockDurationSec, [this, fileId](infrastructure::api::FileLockApi::LockResult lockResult) mutable {
            if (!lockResult) {
                m_statusLabel->clear();
                showApiError("Failed to acquire file lock", lockResult.error());
                return;
            }

            m_openedLockToken = lockResult->lockToken;
            m_releaseButton->setEnabled(true);
            m_lockRenewTimer->start();

            m_statusLabel->setText("Downloading file...");
            m_contentApi.downloadCurrent(*fileId, [this, fileId](infrastructure::api::FileContentApi::BytesResult contentResult) mutable {
                m_statusLabel->clear();
                if (!contentResult) {
                    showApiError("Failed to download file", contentResult.error());
                    releaseOpenedLock();
                    return;
                }

                m_openedFileId = fileId;
                m_openedLogicalName = selectedLogicalName();
                m_editor->setPlainText(QString::fromUtf8(*contentResult));
                setEditorState(true, false, QStringLiteral("Editing: %1").arg(m_openedLogicalName));
                refreshFiles();
            });
        });
    }

    void MainWindow::saveOpenedFile() {
        if (!m_openedFileId || !m_openedLockToken) {
            return;
        }

        m_statusLabel->setText("Saving file...");
        m_contentApi.uploadCurrent(*m_openedFileId, m_editor->toPlainText().toUtf8(), [this](infrastructure::api::FileContentApi::VersionResult result) mutable {
            m_statusLabel->clear();
            if (!result) {
                showApiError("Failed to save file", result.error());
                return;
            }

            statusBar()->showMessage(QStringLiteral("Saved version %1").arg(result->version), 3000);
            refreshFiles();
        });
    }

    void MainWindow::releaseOpenedLock() {
        if (!m_openedFileId || !m_openedLockToken) {
            return;
        }

        const auto fileId = *m_openedFileId;
        const auto lockToken = *m_openedLockToken;
        m_openedLockToken.reset();
        m_lockRenewTimer->stop();
        m_releaseButton->setEnabled(false);
        m_saveButton->setEnabled(false);
        m_editor->setReadOnly(true);

        m_lockApi.release(fileId, lockToken, [this](infrastructure::api::ApiResult<void>) mutable {
            refreshFiles();
        });
    }

    void MainWindow::showVersions() {
        const auto fileId = selectedFileId();
        if (!fileId) {
            return;
        }

        m_versionApi.getAll(*fileId, [this](infrastructure::api::FileVersionApi::VersionsResult result) mutable {
            if (!result) {
                showApiError("Failed to load versions", result.error());
                return;
            }

            QDialog dialog(this);
            dialog.setWindowTitle("File versions");
            dialog.resize(520, 360);

            auto* list = new QListWidget(&dialog);
            for (const auto& version : *result) {
                auto* item = new QListWidgetItem(QStringLiteral("v%1 | id=%2 | %3")
                                                     .arg(version.version)
                                                     .arg(version.id)
                                                     .arg(formatUnixSeconds(version.createdAt)), list);
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
        });
    }

    void MainWindow::openVersion(qint64 versionId) {
        m_contentApi.downloadVersion(versionId, [this, versionId](infrastructure::api::FileContentApi::BytesResult result) mutable {
            if (!result) {
                showApiError("Failed to download version", result.error());
                return;
            }

            m_openedFileId.reset();
            m_openedLogicalName = QStringLiteral("version %1").arg(versionId);
            m_editor->setPlainText(QString::fromUtf8(*result));
            setEditorState(true, true, QStringLiteral("Readonly: version %1").arg(versionId));
        });
    }

    void MainWindow::renewLock() {
        if (!m_openedFileId || !m_openedLockToken) {
            return;
        }

        m_lockApi.renew(*m_openedFileId, *m_openedLockToken, LockDurationSec, [this](infrastructure::api::ApiResult<void> result) mutable {
            if (!result) {
                statusBar()->showMessage("Failed to renew lock", 5000);
                m_lockRenewTimer->stop();
            }
        });
    }

    void MainWindow::logout() {
        if (m_openedFileId && m_openedLockToken) {
            const auto fileId = *m_openedFileId;
            const auto lockToken = *m_openedLockToken;
            m_openedFileId.reset();
            m_openedLockToken.reset();
            m_lockRenewTimer->stop();
            m_lockApi.release(fileId, lockToken, [](infrastructure::api::ApiResult<void>) {});
        }

        m_apiClient.clearBearerToken();
        close();
    }

    std::optional<qint64> MainWindow::selectedFileId() const {
        auto* item = m_fileTree->currentItem();
        if (!item) {
            return std::nullopt;
        }

        const auto value = item->data(0, FileIdRole);
        if (!value.isValid()) {
            return std::nullopt;
        }

        return value.toLongLong();
    }

    QString MainWindow::selectedLogicalName() const {
        const auto fileId = selectedFileId();
        if (!fileId) {
            return {};
        }

        return m_filesById.value(*fileId).fullLogicalName;
    }

    QString MainWindow::aclToText(domain::models::AclLevel aclLevel) {
        switch (aclLevel) {
            case domain::models::AclLevel::NoProperty:
                return "visible";
            case domain::models::AclLevel::ReadOnly:
                return "read only";
            case domain::models::AclLevel::ReadWrite:
                return "read/write";
        }

        return "unknown";
    }

    QString MainWindow::formatUnixSeconds(qint64 seconds) {
        if (seconds <= 0) {
            return {};
        }

        return QDateTime::fromSecsSinceEpoch(seconds).toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
    }

    void MainWindow::setEditorState(bool enabled, bool readOnly, QString title) {
        m_editor->setEnabled(enabled);
        m_editor->setReadOnly(readOnly);
        m_saveButton->setEnabled(enabled && !readOnly && m_openedLockToken.has_value());
        m_releaseButton->setEnabled(m_openedLockToken.has_value());

        if (!title.isEmpty()) {
            statusBar()->showMessage(title, 3000);
        }
    }

    void MainWindow::showApiError(const QString& title, const infrastructure::api::ApiError& error) {
        const auto message = QStringLiteral("%1%2")
            .arg(error.httpStatus > 0 ? QStringLiteral("HTTP %1: ").arg(error.httpStatus) : QStringLiteral("Network error: "))
            .arg(error.message.isEmpty() ? QStringLiteral("unknown error") : error.message);
        QMessageBox::warning(this, title, message);
    }

}
