#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/group.h"

#include <QObject>
#include <QString>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class GroupApi final : public QObject {
        Q_OBJECT

    public:
        using GroupResult = ApiResult<domain::models::Group>;
        using GroupsResult = ApiResult<std::vector<domain::models::Group>>;
        using GroupCallback = std::function<void(GroupResult)>;
        using GroupsCallback = std::function<void(GroupsResult)>;
        using VoidCallback = std::function<void(ApiResult<void>)>;
        using IdListResult = ApiResult<std::vector<qint64>>;
        using IdListCallback = std::function<void(IdListResult)>;

        explicit GroupApi(ApiClient& apiClient, QObject* parent = nullptr);

        void create(const QString& name, GroupCallback callback);
        void getAll(GroupsCallback callback);
        void getById(qint64 groupId, GroupCallback callback);
        void addUser(qint64 userId, qint64 groupId, VoidCallback callback);
        void removeUser(qint64 userId, qint64 groupId, VoidCallback callback);
        void getUserGroups(qint64 userId, IdListCallback callback);
        void getGroupUsers(qint64 groupId, IdListCallback callback);
        void remove(qint64 groupId, VoidCallback callback);

    private:
        static GroupResult parseGroupResponse(const RawApiResponse& response);
        static IdListResult parseIdListResponse(const RawApiResponse& response, const QString& fieldName);

    private:
        ApiClient& m_apiClient;
    };

}
