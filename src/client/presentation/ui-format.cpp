#include "ui-format.h"

#include <QDateTime>

namespace client::presentation {

    QString aclToText(domain::models::AclLevel aclLevel) {
        switch (aclLevel) {
            case domain::models::AclLevel::NoProperty:
                return QStringLiteral("visible");
            case domain::models::AclLevel::ReadOnly:
                return QStringLiteral("read only");
            case domain::models::AclLevel::ReadWrite:
                return QStringLiteral("read/write");
        }

        return QStringLiteral("unknown");
    }

    QString formatUnixSeconds(qint64 seconds) {
        if (seconds <= 0) {
            return {};
        }

        return QDateTime::fromSecsSinceEpoch(seconds).toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    }

}
