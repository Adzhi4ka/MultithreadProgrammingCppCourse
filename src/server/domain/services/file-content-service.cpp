#include "domain/services/file-content-service.h"

#include "infrastructure/file-storage/file-storage.h"
#include "infrastructure/id-generator/id-generator.h"

#include <cerrno>
#include <system_error>

#include <unistd.h>

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
            if (ex.code().value() == ENOENT) {
                return std::unexpected(ServiceError::NotFound);
            }

            return std::unexpected(ServiceError::InternalError);
        }
    }

    ServiceResult<DownloadFileStorage> FileContentService::openDownload(uint64_t physicalPath) {
        try {
            auto storage = FileStorage::openReadOnly(physicalPath);
            const auto fileSize = storage.size();
            if (fileSize < 0) {
                return std::unexpected(ServiceError::InternalError);
            }

            return DownloadFileStorage{.path = FileStorage::makePath(physicalPath),
                                       .size = (uint64_t)fileSize};
        } catch (const std::system_error& ex) {
            if (ex.code().value() == ENOENT) {
                return std::unexpected(ServiceError::NotFound);
            }

            return std::unexpected(ServiceError::InternalError);
        }
    }

    ServiceResult<void> FileContentService::writeAll(uint64_t physicalPath, std::string_view content) {
        try {
            auto storage = FileStorage::openWriteOnly(physicalPath);

            if (::ftruncate(storage.getFd(), 0) == -1) {
                return std::unexpected(ServiceError::InternalError);
            }

            std::size_t totalWritten = 0;
            while (totalWritten < content.size()) {
                const auto writtenBytes = ::write(storage.getFd(),
                                                  content.data() + totalWritten,
                                                  content.size() - totalWritten);

                if (writtenBytes == -1) {
                    if (errno == EINTR) {
                        continue;
                    }

                    return std::unexpected(ServiceError::InternalError);
                }

                if (writtenBytes == 0) {
                    return std::unexpected(ServiceError::InternalError);
                }

                totalWritten += writtenBytes;
            }

            return {};
        } catch (const std::system_error& ex) {
            if (ex.code().value() == ENOENT) {
                return std::unexpected(ServiceError::NotFound);
            }

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