#pragma once

#include "domain/models/remote-file.h"

#include <QWidget>

#include <optional>

class QLabel;

namespace client::presentation {

    class FileInfoWidget : public QWidget {

            Q_OBJECT
            QLabel* m_label = nullptr;

        public:

            explicit FileInfoWidget(QWidget* parent = nullptr);

            void setFile(const std::optional<domain::models::RemoteFile>& file);

    };

}
