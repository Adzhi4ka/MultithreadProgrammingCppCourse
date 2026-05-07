#include "file-info-widget.h"

#include "presentation/ui-format.h"

#include <QLabel>
#include <QVBoxLayout>

namespace client::presentation {

    FileInfoWidget::FileInfoWidget(QWidget* parent)
        : QWidget(parent) {
        m_label = new QLabel(QStringLiteral("Select file"), this);
        m_label->setWordWrap(true);
        m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_label);
    }

    void FileInfoWidget::setFile(const std::optional<domain::models::RemoteFile>& file) {
        if (!file) {
            m_label->setText(QStringLiteral("Select file"));
            return;
        }

        m_label->setText(QStringLiteral(
            "<b>%1</b><br>"
            "ID: %2<br>"
            "Current version ID: %3<br>"
            "Max versions: %4<br>"
            "Created by: %5<br>"
            "Created at: %6<br>"
            "Access: %7<br>"
            "Lock: %8")
            .arg(file->fullLogicalName.toHtmlEscaped())
            .arg(file->id)
            .arg(file->currentVersionId)
            .arg(file->maxVersionCount)
            .arg(file->createdByLogin.isEmpty() ? QStringLiteral("unknown user") : file->createdByLogin)
            .arg(formatUnixSeconds(file->createdAt))
            .arg(aclToText(file->aclLevel))
            .arg(file->hasActiveLock
                     ? QStringLiteral("locked by %1 until %2")
                           .arg(file->lockedByLogin.value_or(QStringLiteral("unknown user")))
                           .arg(formatUnixSeconds(file->lockLeaseUntil.value_or(0)))
                     : QStringLiteral("free")));
    }

}
