#include "create-file-dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <utility>

namespace client::presentation {

    CreateFileDialog::CreateFileDialog(QString currentUserLogin, QWidget* parent) : QDialog(parent) {

        setWindowTitle(QStringLiteral("Create file"));
        setModal(true);
        resize(460, 220);

        m_logicalNameEdit = new QLineEdit(this);
        m_logicalNameEdit->setPlaceholderText(QStringLiteral("/docs/new-file.txt"));
        m_logicalNameEdit->setText(QStringLiteral("/new-file.txt"));

        m_groupCombo = new QComboBox(this);
        m_groupCombo->setEnabled(false);

        m_maxVersionsSpin = new QSpinBox(this);
        m_maxVersionsSpin->setRange(1, 1000);
        m_maxVersionsSpin->setValue(10);

        m_statusLabel = new QLabel(this);
        m_statusLabel->setWordWrap(true);

        auto* formLayout = new QFormLayout();
        formLayout->addRow(QStringLiteral("Logical name"), m_logicalNameEdit);
        formLayout->addRow(QStringLiteral("Create for group"), m_groupCombo);
        formLayout->addRow(QStringLiteral("Max versions"), m_maxVersionsSpin);

        auto* hintLabel = new QLabel(QStringLiteral("The file will be created by %1. The selected group will receive read/write access.")
                                         .arg(std::move(currentUserLogin)),
                                     this);
        hintLabel->setWordWrap(true);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        m_okButton = buttons->button(QDialogButtonBox::Ok);
        m_okButton->setText(QStringLiteral("Create"));
        m_okButton->setEnabled(false);

        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->addWidget(hintLabel);
        rootLayout->addLayout(formLayout);
        rootLayout->addWidget(m_statusLabel);
        rootLayout->addWidget(buttons);

        connect(m_logicalNameEdit, &QLineEdit::textChanged, this, &CreateFileDialog::updateAcceptState);
        connect(m_groupCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &CreateFileDialog::updateAcceptState);
        connect(buttons, &QDialogButtonBox::accepted, this, &CreateFileDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &CreateFileDialog::reject);

        setLoadingGroups();
    }

    QString CreateFileDialog::logicalName() const {
        return m_logicalNameEdit->text().trimmed();
    }

    quint32 CreateFileDialog::maxVersionCount() const {
        return static_cast<quint32>(m_maxVersionsSpin->value());
    }

    qint64 CreateFileDialog::selectedGroupId() const {
        return m_groupCombo->currentData().toLongLong();
    }

    QString CreateFileDialog::selectedGroupName() const {
        return m_groupCombo->currentText();
    }

    void CreateFileDialog::setLoadingGroups() {
        m_groupCombo->clear();
        m_groupCombo->addItem(QStringLiteral("Loading groups..."), qint64{0});
        m_groupCombo->setEnabled(false);
        m_statusLabel->setText(QStringLiteral("Loading available groups..."));
        updateAcceptState();
    }

    void CreateFileDialog::setGroups(ApiResult<std::vector<domain::models::Group>> result) {
        m_groupCombo->clear();

        if (!result) {
            m_groupCombo->addItem(QStringLiteral("Failed to load groups"), qint64{0});
            m_groupCombo->setEnabled(false);
            m_statusLabel->setText(QStringLiteral("Could not load groups: %1").arg(result.error().message));
            updateAcceptState();
            return;
        }

        for (const auto& group : *result) {
            m_groupCombo->addItem(group.name, group.id);
        }

        if (result->empty()) {
            m_groupCombo->addItem(QStringLiteral("No groups available"), qint64{0});
            m_groupCombo->setEnabled(false);
            m_statusLabel->setText(QStringLiteral("You are not a member of any group."));
        } else {
            m_groupCombo->setEnabled(true);
            m_statusLabel->setText(QStringLiteral("Choose the group that should own access to this file."));
        }

        updateAcceptState();
    }

    void CreateFileDialog::updateAcceptState() {
        if (!m_okButton) {
            return;
        }

        const bool hasName = !logicalName().isEmpty();
        const bool hasGroup = m_groupCombo->isEnabled() && selectedGroupId() > 0;
        m_okButton->setEnabled(hasName && hasGroup);
    }

}
