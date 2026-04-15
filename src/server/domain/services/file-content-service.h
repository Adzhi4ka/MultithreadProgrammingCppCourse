// #pragma once

// #include "infrastructure/file-storage/file-storage.h"

// #include <string>

// namespace domain::services {

//     using namespace infrastructure::db;

//     class FileContentService {

//         public:

//             file_storage::FileStorage createNewFileStorage(const std::string& logicalPath) {
//                 return file_storage::FileStorage::createNew(physicalPathFromLogical(logicalPath));
//             }

//             file_storage::FileStorage getFileStorage(const std::string& logicalPath) {
//                 return file_storage::FileStorage::(physicalPathFromLogical(logicalPath));
//             }

//             file_storage::FileStorage deleteFileStorage(const std::string& logicalPath);

//         private:

//             uint64_t physicalPathFromLogical(const std::string& logicalPath);

//     };

// };