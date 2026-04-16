#pragma once

#include "domain/services/service-result.h"

#include "infrastructure/file-storage/file-storage.h"

namespace domain::services {

    using namespace infrastructure::file_storage;

    struct CreatedFileStorage {
        uint64_t physicalPath;
        FileStorage storage;
    };

    class FileContentService {

        public:

            ServiceResult<CreatedFileStorage> createNew();

            ServiceResult<FileStorage> openRead(uint64_t physicalPath);

            ServiceResult<void> remove(uint64_t physicalPath);

        private:

            uint64_t generatePhysicalName();

    };

};