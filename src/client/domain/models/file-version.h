#pragma once

#include <QString>

namespace client::domain::models {

struct FileVersion {
        qint64 id = 0;
        qint64 fileId = 0;
        qint32 version = 0;
        QString logicalNameSnapshot;
        qint64 createdAt = 0;
};

}  // namespace client::domain::models
