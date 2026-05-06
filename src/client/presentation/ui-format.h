#pragma once

#include "domain/models/acl-level.h"

#include <QString>

namespace client::presentation {

    QString aclToText(domain::models::AclLevel aclLevel);
    QString formatUnixSeconds(qint64 seconds);

}
