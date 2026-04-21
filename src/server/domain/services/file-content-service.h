#pragma once

#include "domain/services/service-result.h"

#include "infrastructure/file-storage/file-storage.h"

namespace domain::services {

    struct CreatedFileStorage {
        uint64_t physicalPath;
        infrastructure::file_storage::FileStorage storage;
    };

    class FileContentService {

        public:

            ServiceResult<CreatedFileStorage> createNew();

            ServiceResult<infrastructure::file_storage::FileStorage> openRead(uint64_t physicalPath);

            ServiceResult<void> remove(uint64_t physicalPath);

        private:

            uint64_t generatePhysicalName();

    };

} // namespace domain::services