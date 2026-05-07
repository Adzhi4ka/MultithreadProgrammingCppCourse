#include "file-tree-widget.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QRegularExpression>

#include "presentation/ui-format.h"

namespace client::presentation {

namespace {
constexpr int FileIdRole = Qt::UserRole + 1;
}

FileTreeWidget::FileTreeWidget(QWidget* parent) : QTreeWidget(parent) {
    setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Access"), QStringLiteral("Lock"),
                     QStringLiteral("Created by"), QStringLiteral("Created at")});
    setAlternatingRowColors(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::SingleSelection);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    connect(this, &QTreeWidget::itemSelectionChanged, this, &FileTreeWidget::emitSelection);
    connect(this, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int) {
        if (!item) {
            return;
        }

        const auto value = item->data(0, FileIdRole);
        if (value.isValid()) {
            emit fileActivated(value.toLongLong());
        }
    });
}

void FileTreeWidget::setFiles(const std::vector<domain::models::RemoteFile>& files) {
    clear();
    m_filesById.clear();
    m_folderItemsByPath.clear();

    for (const auto& file : files) {
        m_filesById.insert(file.id, file);
        insertFileItem(file);
    }

    expandToDepth(1);
    emitSelection();
}

std::optional<qint64> FileTreeWidget::selectedFileId() const {
    const auto* item = currentItem();
    if (!item) {
        return std::nullopt;
    }

    const auto value = item->data(0, FileIdRole);
    if (!value.isValid()) {
        return std::nullopt;
    }

    return value.toLongLong();
}

std::optional<domain::models::RemoteFile> FileTreeWidget::selectedFile() const {
    const auto fileId = selectedFileId();
    if (!fileId) {
        return std::nullopt;
    }

    return fileById(*fileId);
}

std::optional<domain::models::RemoteFile> FileTreeWidget::fileById(qint64 fileId) const {
    const auto it = m_filesById.constFind(fileId);
    if (it == m_filesById.cend()) {
        return std::nullopt;
    }

    return it.value();
}

qsizetype FileTreeWidget::fileCount() const noexcept { return m_filesById.size(); }

void FileTreeWidget::insertFileItem(const domain::models::RemoteFile& file) {
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

    auto* item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
    item->setText(0, parts.constLast());
    item->setText(1, aclToText(file.aclLevel));
    item->setText(2,
                  file.hasActiveLock
                      ? QStringLiteral("locked by %1").arg(file.lockedByLogin.value_or(QStringLiteral("unknown user")))
                      : QStringLiteral("free"));
    item->setText(3, file.createdByLogin.isEmpty() ? QStringLiteral("unknown user") : file.createdByLogin);
    item->setText(4, formatUnixSeconds(file.createdAt));
    item->setData(0, FileIdRole, file.id);
}

QTreeWidgetItem* FileTreeWidget::ensureFolderItem(const QString& pathPart, QTreeWidgetItem* parent,
                                                  const QString& fullPath) {
    if (auto it = m_folderItemsByPath.find(fullPath); it != m_folderItemsByPath.end()) {
        return it.value();
    }

    auto* item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
    item->setText(0, pathPart);
    m_folderItemsByPath.insert(fullPath, item);
    return item;
}

void FileTreeWidget::emitSelection() { emit selectedFileChanged(); }

}  // namespace client::presentation
