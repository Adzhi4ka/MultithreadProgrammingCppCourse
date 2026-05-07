#pragma once

#include <QString>

#include "domain/models/acl-level.h"

namespace client::presentation {

QString aclToText(domain::models::AclLevel aclLevel);
QString formatUnixSeconds(qint64 seconds);

}  // namespace client::presentation
