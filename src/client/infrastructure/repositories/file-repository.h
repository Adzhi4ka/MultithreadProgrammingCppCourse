#pragma once

#include <QHash>
#include <optional>
#include <vector>

#include "domain/models/remote-file.h"

namespace client::infrastructure::repositories {

class FileRepository {

        using RemoteFile = domain::models::RemoteFile;

        QHash<qint64, RemoteFile> m_filesById;

    public:

        void replaceAll(std::vector<RemoteFile> files);
        void upsert(RemoteFile file);
        void remove(qint64 fileId);

        std::optional<RemoteFile> findById(qint64 fileId) const;
        std::vector<RemoteFile> all() const;
};

}  // namespace client::infrastructure::repositories
