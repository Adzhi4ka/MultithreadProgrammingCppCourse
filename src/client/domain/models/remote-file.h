#pragma once

#include "acl-level.h"

#include <QString>
#include <optional>

namespace client::domain::models {

    struct RemoteFile {
        qint64 id = 0;
        QString fullLogicalName;
        qint64 currentVersionId = 0;
        quint32 maxVersionCount = 0;
        qint64 createdAt = 0;
        qint64 createdBy = 0;
        QString createdByLogin;

        AclLevel aclLevel = AclLevel::NoProperty;
        bool hasActiveLock = false;
        std::optional<qint64> lockedByUserId;
        std::optional<QString> lockedByLogin;
        std::optional<qint64> lockLeaseUntil;
    };

}
