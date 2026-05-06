#pragma once

#include <QJsonObject>
#include <QString>

namespace client::domain::models {

    struct NotificationEvent {
        QString name;
        QJsonObject payload;
    };

}
