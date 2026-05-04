#pragma once

#include "acl-level.h"

namespace client::domain::models {

    struct FileAcl {
        qint64 fileId = 0;
        qint64 groupId = 0;
        AclLevel aclLevel = AclLevel::NoProperty;
    };

    struct UserFileAcl {
        qint64 fileId = 0;
        qint64 userId = 0;
        AclLevel aclLevel = AclLevel::NoProperty;
    };

}
