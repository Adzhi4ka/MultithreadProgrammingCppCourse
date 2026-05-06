#pragma once

#include "domain/models/remote-file.h"

#include <QHash>
#include <QTreeWidget>

#include <optional>
#include <vector>

namespace client::presentation {

    class FileTreeWidget : public QTreeWidget {

            Q_OBJECT
            QHash<qint64, domain::models::RemoteFile> m_filesById;
            QHash<QString, QTreeWidgetItem*> m_folderItemsByPath;

        public:

            explicit FileTreeWidget(QWidget* parent = nullptr);

            void setFiles(const std::vector<domain::models::RemoteFile>& files);
            std::optional<qint64> selectedFileId() const;
            std::optional<domain::models::RemoteFile> selectedFile() const;
            std::optional<domain::models::RemoteFile> fileById(qint64 fileId) const;
            qsizetype fileCount() const noexcept;

        signals:

            void selectedFileChanged();
            void fileActivated(qint64 fileId);

        private:

            void insertFileItem(const domain::models::RemoteFile& file);
            QTreeWidgetItem* ensureFolderItem(const QString& pathPart, QTreeWidgetItem* parent, const QString& fullPath);
            void emitSelection();

    };

}
