#include "domain/services/file-content-service.h"

#include "infrastructure/file-storage/file-storage.h"
#include "infrastructure/id-generator/id-generator.h"

namespace {
    using namespace infrastructure::file_storage;
}

namespace domain::services {

    ServiceResult<CreatedFileStorage> FileContentService::createNew() {

        auto physicalPath = generatePhysicalName();
        try {
            auto newFileStorage = FileStorage::createNew(physicalPath);
            return CreatedFileStorage{.physicalPath = physicalPath,
                                      .storage = std::move(newFileStorage)};
        } catch (const std::system_error& ex) {
            if (ex.code().value() == EEXIST) {
                return std::unexpected(ServiceError::Conflict);
            }

            return std::unexpected(ServiceError::InternalError);
        }
    }

    ServiceResult<FileStorage> FileContentService::openRead(uint64_t physicalPath) {
        try {
            return FileStorage::openReadOnly(physicalPath);
        } catch (const std::system_error& ex) {
            return std::unexpected(ServiceError::InternalError);
        }
    }

    ServiceResult<void> FileContentService::remove(uint64_t physicalPath) {
        try {
            FileStorage::remove(physicalPath);
            return {};
        } catch (const std::system_error& ex) {
            return std::unexpected(ServiceError::InternalError);
        }
    }

    uint64_t FileContentService::generatePhysicalName() {
        return infrastructure::id_generator::generateId();
    }

}