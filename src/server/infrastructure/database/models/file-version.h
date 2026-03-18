#pragma once

#include <cstdint>
#include <string>

namespace infrastructure::db::models {

    struct FileVersion {
        int64_t id;
        int64_t fileId;
        uint32_t version;
        std::string logicalNameSnapshot;
        uint64_t physicalPathName;
        int64_t createdAt;
    };

}