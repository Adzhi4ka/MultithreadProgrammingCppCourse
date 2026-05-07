#pragma once

#include <QWidget>
#include <optional>

#include "domain/models/remote-file.h"

class QLabel;

namespace client::presentation {

class FileInfoWidget : public QWidget {

        Q_OBJECT
        QLabel* m_label = nullptr;

    public:

        explicit FileInfoWidget(QWidget* parent = nullptr);

        void setFile(const std::optional<domain::models::RemoteFile>& file);
};

}  // namespace client::presentation
