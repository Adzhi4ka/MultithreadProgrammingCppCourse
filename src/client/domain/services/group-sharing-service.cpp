#include "group-sharing-service.h"

#include <algorithm>
#include <expected>
#include <memory>
#include <utility>

namespace client::domain::services {

struct GroupSharingService::GroupLoadState {
        std::vector<Group> groups;
        int pending = 0;
        std::shared_ptr<std::function<void(ApiResult<std::vector<Group>>)>> callback;
};

struct GroupSharingService::UserLoadState {
        qint64 groupId = 0;
        std::vector<UserProfile> users;
        int pending = 0;
        std::shared_ptr<std::function<void(ApiResult<std::vector<UserProfile>>)>> callback;
};

GroupSharingService::GroupSharingService(application::NetworkWorker& networkWorker, QObject& internalContext,
                                         QObject& uiContext,
                                         infrastructure::repositories::GroupRepository& groupRepository) noexcept
    : RemoteServiceBase(networkWorker, internalContext, uiContext), m_groupRepository(groupRepository) {}

void GroupSharingService::loadCurrentUserGroups(qint64 currentUserId,
                                                std::function<void(ApiResult<std::vector<Group>>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<std::vector<Group>>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([currentUserId, internalContext, cb, service](RemoteApiGateway& gateway) mutable {
        gateway.groupApi().getUserGroups(
            currentUserId, [internalContext, cb, service](ApiResult<std::vector<qint64>> result) mutable {
                ::client::application::postTask(internalContext, [cb, service, result = std::move(result)]() mutable {
                    service->loadGroupsByIds(std::move(result), std::move(cb));
                });
            });
    });
}

void GroupSharingService::loadGroupUsers(qint64 groupId,
                                         std::function<void(ApiResult<std::vector<UserProfile>>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<std::vector<UserProfile>>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([groupId, internalContext, cb, service](RemoteApiGateway& gateway) mutable {
        gateway.groupApi().getGroupUsers(
            groupId, [groupId, internalContext, cb, service](ApiResult<std::vector<qint64>> result) mutable {
                ::client::application::postTask(internalContext,
                                                [groupId, cb, service, result = std::move(result)]() mutable {
                                                    service->loadUsersByIds(groupId, std::move(result), std::move(cb));
                                                });
            });
    });
}

void GroupSharingService::addUserToGroup(QString login, qint64 groupId, std::function<void(ApiResult<void>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<void>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    QPointer<QObject> uiContext{&m_uiContext};
    auto* groupRepository = &m_groupRepository;

    m_networkWorker.run([login = std::move(login), groupId, internalContext, uiContext, cb,
                         groupRepository](RemoteApiGateway& gateway) mutable {
        auto* gatewayPtr = &gateway;
        gateway.userApi().getByLogin(login, [groupId, internalContext, uiContext, cb, groupRepository,
                                             gatewayPtr](ApiResult<UserProfile> userResult) mutable {
            if (!userResult) {
                ::client::application::postTask(
                    internalContext, [uiContext, cb, userResult = std::move(userResult)]() mutable {
                        ::client::application::postResult(uiContext, cb,
                                                          ApiResult<void>{std::unexpected(userResult.error())});
                    });
                return;
            }

            const auto user = *userResult;
            gatewayPtr->groupApi().addUser(
                user.userId, groupId,
                [user, groupId, internalContext, uiContext, cb, groupRepository](ApiResult<void> result) mutable {
                    ::client::application::postTask(internalContext, [user, groupId, uiContext, cb, groupRepository,
                                                                      result = std::move(result)]() mutable {
                        if (result) {
                            auto users = groupRepository->groupUsers(groupId);
                            users.emplace_back(user);
                            groupRepository->upsertUser(user);
                            groupRepository->replaceGroupUsers(groupId, std::move(users));
                        }

                        ::client::application::postResult(uiContext, cb, std::move(result));
                    });
                });
        });
    });
}

void GroupSharingService::removeUserFromGroup(qint64 userId, qint64 groupId,
                                              std::function<void(ApiResult<void>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<void>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    QPointer<QObject> uiContext{&m_uiContext};
    auto* groupRepository = &m_groupRepository;

    m_networkWorker.run(
        [userId, groupId, internalContext, uiContext, cb, groupRepository](RemoteApiGateway& gateway) mutable {
            gateway.groupApi().removeUser(
                userId, groupId,
                [userId, groupId, internalContext, uiContext, cb, groupRepository](ApiResult<void> result) mutable {
                    ::client::application::postTask(internalContext, [userId, groupId, uiContext, cb, groupRepository,
                                                                      result = std::move(result)]() mutable {
                        if (result) {
                            auto users = groupRepository->groupUsers(groupId);
                            users.erase(std::remove_if(users.begin(), users.end(),
                                                       [userId](const auto& user) { return user.userId == userId; }),
                                        users.end());
                            groupRepository->replaceGroupUsers(groupId, std::move(users));
                        }

                        ::client::application::postResult(uiContext, cb, std::move(result));
                    });
                });
        });
}

void GroupSharingService::createGroupForCurrentUser(QString name, qint64 currentUserId,
                                                    std::function<void(ApiResult<Group>)> callback) {
    auto cb = std::make_shared<std::function<void(ApiResult<Group>)>>(std::move(callback));
    QPointer<QObject> internalContext{&m_internalContext};
    QPointer<QObject> uiContext{&m_uiContext};
    auto* groupRepository = &m_groupRepository;

    m_networkWorker.run([name = std::move(name), currentUserId, internalContext, uiContext, cb,
                         groupRepository](RemoteApiGateway& gateway) mutable {
        auto* gatewayPtr = &gateway;
        gateway.groupApi().create(name, [currentUserId, internalContext, uiContext, cb, groupRepository,
                                         gatewayPtr](ApiResult<Group> groupResult) mutable {
            if (!groupResult) {
                ::client::application::postTask(
                    internalContext, [uiContext, cb, groupResult = std::move(groupResult)]() mutable {
                        ::client::application::postResult(uiContext, cb, std::move(groupResult));
                    });
                return;
            }

            const auto group = *groupResult;
            gatewayPtr->groupApi().addUser(
                currentUserId, group.id,
                [currentUserId, group, internalContext, uiContext, cb,
                 groupRepository](ApiResult<void> addResult) mutable {
                    ::client::application::postTask(
                        internalContext, [currentUserId, group, uiContext, cb, groupRepository,
                                          addResult = std::move(addResult)]() mutable {
                            if (!addResult) {
                                ::client::application::postResult(uiContext, cb,
                                                                  ApiResult<Group>{std::unexpected(addResult.error())});
                                return;
                            }

                            groupRepository->upsertGroup(group);
                            if (auto currentUser = groupRepository->findUser(currentUserId)) {
                                groupRepository->replaceGroupUsers(group.id, {*currentUser});
                            }
                            ::client::application::postResult(uiContext, cb, apiSuccess(group));
                        });
                });
        });
    });
}

void GroupSharingService::loadGroupsByIds(
    ApiResult<std::vector<qint64>> idsResult,
    std::shared_ptr<std::function<void(ApiResult<std::vector<Group>>)>> callback) {
    if (!idsResult) {
        postUi(std::move(callback), ApiResult<std::vector<Group>>{std::unexpected(idsResult.error())});
        return;
    }

    auto state = std::make_shared<GroupLoadState>();
    state->pending = idsResult->size();
    state->groups.reserve(idsResult->size());
    state->callback = std::move(callback);

    if (state->pending == 0) {
        m_groupRepository.replaceGroups({});
        postUi(state->callback, apiSuccess(m_groupRepository.groups()));
        return;
    }

    for (const auto groupId : *idsResult) {
        requestGroup(groupId, state);
    }
}

void GroupSharingService::requestGroup(qint64 groupId, std::shared_ptr<GroupLoadState> state) {
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([groupId, state, internalContext, service](RemoteApiGateway& gateway) mutable {
        gateway.groupApi().getById(
            groupId, [groupId, state, internalContext, service](ApiResult<Group> result) mutable {
                ::client::application::postTask(
                    internalContext, [groupId, state, service, result = std::move(result)]() mutable {
                        if (result) {
                            state->groups.emplace_back(std::move(*result));
                        } else {
                            state->groups.emplace_back(Group{.id = groupId, .name = QStringLiteral("unknown group")});
                        }

                        --state->pending;
                        if (state->pending > 0) {
                            return;
                        }

                        std::sort(state->groups.begin(), state->groups.end(), [](const auto& lhs, const auto& rhs) {
                            return lhs.name.localeAwareCompare(rhs.name) < 0;
                        });

                        service->m_groupRepository.replaceGroups(state->groups);
                        service->postUi(state->callback, apiSuccess(service->m_groupRepository.groups()));
                    });
            });
    });
}

void GroupSharingService::loadUsersByIds(
    qint64 groupId, ApiResult<std::vector<qint64>> idsResult,
    std::shared_ptr<std::function<void(ApiResult<std::vector<UserProfile>>)>> callback) {
    if (!idsResult) {
        postUi(std::move(callback), ApiResult<std::vector<UserProfile>>{std::unexpected(idsResult.error())});
        return;
    }

    auto state = std::make_shared<UserLoadState>();
    state->groupId = groupId;
    state->pending = idsResult->size();
    state->users.reserve(idsResult->size());
    state->callback = std::move(callback);

    if (state->pending == 0) {
        m_groupRepository.replaceGroupUsers(groupId, {});
        postUi(state->callback, apiSuccess(m_groupRepository.groupUsers(groupId)));
        return;
    }

    for (const auto userId : *idsResult) {
        requestUser(groupId, userId, state);
    }
}

void GroupSharingService::requestUser(qint64 groupId, qint64 userId, std::shared_ptr<UserLoadState> state) {
    QPointer<QObject> internalContext{&m_internalContext};
    auto* service = this;

    m_networkWorker.run([groupId, userId, state, internalContext, service](RemoteApiGateway& gateway) mutable {
        gateway.userApi().getById(userId, [groupId, userId, state, internalContext,
                                           service](ApiResult<UserProfile> result) mutable {
            ::client::application::postTask(internalContext, [groupId, userId, state, service,
                                                              result = std::move(result)]() mutable {
                if (result) {
                    state->users.emplace_back(std::move(*result));
                } else {
                    state->users.emplace_back(UserProfile{.userId = userId, .login = QStringLiteral("unknown user")});
                }

                --state->pending;
                if (state->pending > 0) {
                    return;
                }

                service->m_groupRepository.replaceGroupUsers(groupId, std::move(state->users));
                service->postUi(state->callback, apiSuccess(service->m_groupRepository.groupUsers(groupId)));
            });
        });
    });
}

}  // namespace client::domain::services
