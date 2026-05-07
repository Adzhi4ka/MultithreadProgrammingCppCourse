#pragma once

#include "domain/models/user-profile.h"
#include "domain/services/remote-service-base.h"
#include "infrastructure/repositories/group-repository.h"

#include <functional>
#include <memory>
#include <vector>

namespace client::domain::services {

    class GroupSharingService : public RemoteServiceBase {

            using Group = domain::models::Group;
            using UserProfile = domain::models::UserProfile;

            infrastructure::repositories::GroupRepository& m_groupRepository;

        public:
            GroupSharingService(application::NetworkWorker& networkWorker,
                                QObject& internalContext,
                                QObject& uiContext,
                                infrastructure::repositories::GroupRepository& groupRepository) noexcept;

            void loadCurrentUserGroups(qint64 currentUserId,
                                       std::function<void(ApiResult<std::vector<Group>>)> callback);
            void loadGroupUsers(qint64 groupId,
                                std::function<void(ApiResult<std::vector<UserProfile>>)> callback);
            void addUserToGroup(QString login,
                                qint64 groupId,
                                std::function<void(ApiResult<void>)> callback);
            void removeUserFromGroup(qint64 userId,
                                    qint64 groupId,
                                    std::function<void(ApiResult<void>)> callback);
            void createGroupForCurrentUser(QString name,
                                        qint64 currentUserId,
                                        std::function<void(ApiResult<domain::models::Group>)> callback);

        private:
            struct GroupLoadState;
            struct UserLoadState;

            void loadGroupsByIds(ApiResult<std::vector<qint64>> idsResult,
                                std::shared_ptr<std::function<void(ApiResult<std::vector<domain::models::Group>>)>> callback);
            void requestGroup(qint64 groupId, std::shared_ptr<GroupLoadState> state);
            void loadUsersByIds(qint64 groupId,
                                ApiResult<std::vector<qint64>> idsResult,
                                std::shared_ptr<std::function<void(ApiResult<std::vector<UserProfile>>)>> callback);
            void requestUser(qint64 groupId, qint64 userId, std::shared_ptr<UserLoadState> state);

    };

}
