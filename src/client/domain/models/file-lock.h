#pragma once

#include "qtypes.h"

namespace client::domain::models {

struct FileLock {
        qint64 fileId = 0;
        qint64 userId = 0;
        qint64 leaseUntil = 0;
        qint64 lockToken = 0;
};

}  // namespace client::domain::models
