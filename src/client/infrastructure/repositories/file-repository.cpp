#include "file-repository.h"

#include <algorithm>
#include <utility>

namespace {
    using RemoteFile = client::domain::models::RemoteFile;
}

namespace client::infrastructure::repositories {

    void FileRepository::replaceAll(std::vector<RemoteFile> files) {
        m_filesById.clear();
        for (auto& file : files) {
            m_filesById.insert(file.id, std::move(file));
        }
    }

    void FileRepository::upsert(RemoteFile file) {
        m_filesById.insert(file.id, std::move(file));
    }

    void FileRepository::remove(qint64 fileId) {
        m_filesById.remove(fileId);
    }

    std::optional<RemoteFile> FileRepository::findById(qint64 fileId) const {
        const auto it = m_filesById.constFind(fileId);
        if (it == m_filesById.constEnd()) {
            return std::nullopt;
        }

        return *it;
    }

    std::vector<RemoteFile> FileRepository::all() const {
        std::vector<RemoteFile> files;
        files.reserve(m_filesById.size());

        for (auto it = m_filesById.constBegin(); it != m_filesById.constEnd(); ++it) {
            files.emplace_back(*it);
        }

        std::sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.fullLogicalName.localeAwareCompare(rhs.fullLogicalName) < 0;
        });

        return files;
    }

}
