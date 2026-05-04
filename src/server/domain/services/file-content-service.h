#pragma once

#include "domain/services/service-result.h"

#include "infrastructure/file-storage/file-storage.h"

#include <string>
#include <string_view>

namespace domain::services {

    struct CreatedFileStorage {
        uint64_t physicalPath;
        infrastructure::file_storage::FileStorage storage;
    };

    struct DownloadFileStorage {
        std::string path;
        uint64_t size;
    };

    class FileContentService {

        public:

            ServiceResult<CreatedFileStorage> createNew();

            ServiceResult<infrastructure::file_storage::FileStorage> openRead(uint64_t physicalPath);

            ServiceResult<DownloadFileStorage> openDownload(uint64_t physicalPath);

            ServiceResult<void> writeAll(uint64_t physicalPath, std::string_view content);

            ServiceResult<void> remove(uint64_t physicalPath);

        private:

            uint64_t generatePhysicalName();

    };

} // namespace domain::services