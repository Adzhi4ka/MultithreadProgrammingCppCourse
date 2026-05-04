#pragma once

#include <QString>

namespace client::domain::models {

    enum class AclLevel : qint8 {
        NoProperty = 0,
        ReadOnly = 1,
        ReadWrite = 2,
    };

    inline QString toServerString(AclLevel level) {
        switch (level) {
            case AclLevel::NoProperty:
                return "no_property";
            case AclLevel::ReadOnly:
                return "read_only";
            case AclLevel::ReadWrite:
                return "read_write";
        }

        return "no_property";
    }

    inline AclLevel aclLevelFromServerString(const QString& value) {
        if (value == "read_only" || value == "READ_ONLY") {
            return AclLevel::ReadOnly;
        }

        if (value == "read_write" || value == "READ_WRITE") {
            return AclLevel::ReadWrite;
        }

        return AclLevel::NoProperty;
    }

    inline bool canRead(AclLevel level) noexcept {
        return level == AclLevel::ReadOnly || level == AclLevel::ReadWrite;
    }

    inline bool canWrite(AclLevel level) noexcept {
        return level == AclLevel::ReadWrite;
    }

}
