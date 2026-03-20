#pragma once

#include <cstdint>
#include <optional>

namespace domain::services {

    class FileLockService {

            static constexpr int64_t defaultLockDuration = 3600;
        
        public:

            bool acquireLock(int64_t fileId, int64_t userId, int64_t lockDuration = defaultLockDuration);

            void renewLock(int64_t fileId, int64_t lockDuration = defaultLockDuration);

            void releaseLock(int64_t fileId);

            std::optional<FileLock> getActiveLock(int64_t fileId);

    };

};