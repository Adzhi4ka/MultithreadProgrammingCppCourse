#pragma once

#include "domain/models/group.h"
#include "infrastructure/api/api-result.h"

#include <QDialog>
#include <QString>
#include <QtGlobal>

#include <vector>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace client::presentation {

    class CreateFileDialog : public QDialog {

            Q_OBJECT

            QLineEdit* m_logicalNameEdit = nullptr;
            QComboBox* m_groupCombo = nullptr;
            QSpinBox* m_maxVersionsSpin = nullptr;
            QLabel* m_statusLabel = nullptr;
            QPushButton* m_okButton = nullptr;

        public:

            explicit CreateFileDialog(QString currentUserLogin, QWidget* parent = nullptr);

            QString logicalName() const;
            quint32 maxVersionCount() const;
            qint64 selectedGroupId() const;
            QString selectedGroupName() const;

            void setLoadingGroups();
            void setGroups(ApiResult<std::vector<domain::models::Group>> result);

        private slots:

            void updateAcceptState();

    };

}
