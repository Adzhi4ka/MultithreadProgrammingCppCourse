#pragma once

#include "api-client.h"
#include "api-result.h"
#include "domain/models/group.h"

#include <QObject>
#include <QString>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class GroupApi : public QObject {

            using Group = domain::models::Group;

            Q_OBJECT
            ApiClient& m_apiClient;

        public:

            explicit GroupApi(ApiClient& apiClient, QObject* parent = nullptr);

            void create(const QString& name, std::function<void(ApiResult<Group>)> callback);
            void getAll(std::function<void(ApiResult<std::vector<Group>>)> callback);
            void getById(qint64 groupId, std::function<void(ApiResult<Group>)> callback);
            void addUser(qint64 userId, qint64 groupId, std::function<void(ApiResult<void>)> callback);
            void removeUser(qint64 userId, qint64 groupId, std::function<void(ApiResult<void>)> callback);
            void getUserGroups(qint64 userId, std::function<void(ApiResult<std::vector<qint64>>)> callback);
            void getGroupUsers(qint64 groupId, std::function<void(ApiResult<std::vector<qint64>>)> callback);
            void remove(qint64 groupId, std::function<void(ApiResult<void>)> callback);

        private:

            static ApiResult<Group> parseGroupResponse(const RawApiResponse& response);
            static ApiResult<std::vector<qint64>> parseIdListResponse(const RawApiResponse& response,
                                                                      const QString& fieldName);

    };

}
