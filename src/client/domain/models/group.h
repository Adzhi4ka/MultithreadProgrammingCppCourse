#pragma once

#include <QString>

namespace client::domain::models {

struct Group {
        qint64 id = 0;
        QString name;
};

}  // namespace client::domain::models
