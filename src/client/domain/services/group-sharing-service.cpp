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

    GroupSharingService::GroupSharingService(application::NetworkWorker& networkWorker,
                                             QObject& internalContext,
                                             QObject& uiContext,
                                             infrastructure::repositories::GroupRepository& groupRepository) noexcept
        : RemoteServiceBase(networkWorker, internalContext, uiContext),
          m_groupRepository(groupRepository) {}

    void GroupSharingService::loadCurrentUserGroups(qint64 currentUserId, std::function<void(ApiResult<std::vector<Group>>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<std::vector<Group>>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        auto* service = this;

        m_networkWorker.run([currentUserId, internalContext, cb, service](RemoteApiGateway& gateway) mutable {
            gateway.groupApi().getUserGroups(currentUserId, [internalContext, cb, service](ApiResult<std::vector<qint64>> result) mutable {
                ::client::application::postTask(internalContext,
                                      [cb, service, result = std::move(result)]() mutable {
                                          service->loadGroupsByIds(std::move(result), std::move(cb));
                                      });
            });
        });
    }

    void GroupSharingService::loadGroupUsers(qint64 groupId, std::function<void(ApiResult<std::vector<qint64>>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<std::vector<qint64>>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* groupRepository = &m_groupRepository;

        m_networkWorker.run([groupId,
                             internalContext,
                             uiContext,
                             cb,
                             groupRepository](RemoteApiGateway& gateway) mutable {
            gateway.groupApi().getGroupUsers(groupId, [groupId, internalContext, uiContext, cb, groupRepository](ApiResult<std::vector<qint64>> result) mutable {
                ::client::application::postTask(internalContext,
                                      [groupId, uiContext, cb, groupRepository, result = std::move(result)]() mutable {
                                          if (result) {
                                              groupRepository->replaceGroupUsers(groupId, *result);
                                          }

                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            });
        });
    }

    void GroupSharingService::addUserToGroup(qint64 userId, qint64 groupId, std::function<void(ApiResult<void>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<void>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* groupRepository = &m_groupRepository;

        m_networkWorker.run([userId,
                             groupId,
                             internalContext,
                             uiContext,
                             cb,
                             groupRepository](RemoteApiGateway& gateway) mutable {
            gateway.groupApi().addUser(userId, groupId, [userId, groupId, internalContext, uiContext, cb, groupRepository](ApiResult<void> result) mutable {
                ::client::application::postTask(internalContext,
                                      [userId, groupId, uiContext, cb, groupRepository, result = std::move(result)]() mutable {
                                          if (result) {
                                              auto users = groupRepository->groupUsers(groupId);
                                              users.emplace_back(userId);
                                              groupRepository->replaceGroupUsers(groupId, std::move(users));
                                          }

                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            });
        });
    }

    void GroupSharingService::removeUserFromGroup(qint64 userId, qint64 groupId, std::function<void(ApiResult<void>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<void>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* groupRepository = &m_groupRepository;

        m_networkWorker.run([userId,
                             groupId,
                             internalContext,
                             uiContext,
                             cb,
                             groupRepository](RemoteApiGateway& gateway) mutable {
            gateway.groupApi().removeUser(userId, groupId, [userId, groupId, internalContext, uiContext, cb, groupRepository](ApiResult<void> result) mutable {
                ::client::application::postTask(internalContext,
                                      [userId, groupId, uiContext, cb, groupRepository, result = std::move(result)]() mutable {
                                          if (result) {
                                              auto users = groupRepository->groupUsers(groupId);
                                              users.erase(std::remove(users.begin(), users.end(), userId), users.end());
                                              groupRepository->replaceGroupUsers(groupId, std::move(users));
                                          }

                                          ::client::application::postResult(uiContext, cb, std::move(result));
                                      });
            });
        });
    }

    void GroupSharingService::createGroupForCurrentUser(QString name,
                                                        qint64 currentUserId,
                                                        std::function<void(ApiResult<Group>)> callback) {
        auto cb = std::make_shared<std::function<void(ApiResult<Group>)>>(std::move(callback));
        QPointer<QObject> internalContext{&m_internalContext};
        QPointer<QObject> uiContext{&m_uiContext};
        auto* groupRepository = &m_groupRepository;

        m_networkWorker.run([name = std::move(name),
                             currentUserId,
                             internalContext,
                             uiContext,
                             cb,
                             groupRepository](RemoteApiGateway& gateway) mutable {
            auto* gatewayPtr = &gateway;
            gateway.groupApi().create(name, [currentUserId, internalContext, uiContext, cb, groupRepository, gatewayPtr](ApiResult<Group> groupResult) mutable {
                if (!groupResult) {
                    ::client::application::postTask(internalContext,
                                          [uiContext, cb, groupResult = std::move(groupResult)]() mutable {
                                              ::client::application::postResult(uiContext, cb, std::move(groupResult));
                                          });
                    return;
                }

                const auto group = *groupResult;
                gatewayPtr->groupApi().addUser(currentUserId, group.id, [currentUserId, group, internalContext, uiContext, cb, groupRepository](ApiResult<void> addResult) mutable {
                    ::client::application::postTask(internalContext,
                                          [currentUserId, group, uiContext, cb, groupRepository, addResult = std::move(addResult)]() mutable {
                                              if (!addResult) {
                                                  ::client::application::postResult(uiContext, cb, ApiResult<Group>{std::unexpected(addResult.error())});
                                                  return;
                                              }

                                              groupRepository->upsertGroup(group);
                                              groupRepository->replaceGroupUsers(group.id, {currentUserId});
                                              ::client::application::postResult(uiContext, cb, apiSuccess(group));
                                          });
                });
            });
        });
    }

    void GroupSharingService::loadGroupsByIds(ApiResult<std::vector<qint64>> idsResult,
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
            gateway.groupApi().getById(groupId, [groupId, state, internalContext, service](ApiResult<Group> result) mutable {
                ::client::application::postTask(internalContext,
                                      [groupId, state, service, result = std::move(result)]() mutable {
                                          if (result) {
                                              state->groups.emplace_back(std::move(*result));
                                          } else {
                                              state->groups.emplace_back(Group{.id = groupId, .name = QStringLiteral("#%1").arg(groupId)});
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

}
