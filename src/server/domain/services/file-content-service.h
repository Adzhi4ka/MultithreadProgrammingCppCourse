#pragma once

#include <string>

namespace domain::services {

    class FileContentService {



        public:

            FileStorage createNewFileStorage(const std::string& logicalPath);

            FileStorage getFileStorage(const std::string& logicalPath);

            FileStorage deleteFileStorage(const std::string& logicalPath);

        private:

            uint64_t physicalPathFromLogical(const std::string& logicalPath);

    };

};