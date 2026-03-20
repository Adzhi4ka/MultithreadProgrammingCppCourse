#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace domain::service {

    class FileService {


        public:

            int64_t createFile(const std::string& logicalName, int64_t createdByUser);

            std::optional<File> getFileById(int64_t fileId);

            std::optional<File> getFileByLogicalName(const std::string& name);

            std::optional<FileVersion> getCurrentVersion(int64_t fileId);
            std::vector<FileVersion> getAllVersions(int64_t fileId);

            void rename(int64_t fileId, const std::string& newLogicalName);

            int64_t createNewVersion(int64_t fileId, const std::string&, uint64_t physicalKey);

            struct ForkResult {int64_t newFileId; int64_t newVersionId;};
            ForkResult forkFromOldVersion(int64_t sourceFileId, uint32_t sourceVersionNumber, const std::string& newLogicalName, int64_t createdBy);

            void deleteFile(int64_t fileId);

            std::vector<uint64_t> getAllUsedPhysicalKeys() const;
    };

};