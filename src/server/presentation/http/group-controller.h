#pragma once

#include "domain/notifications/notification-publisher.h"
#include "domain/services/group-service.h"
#include "infrastructure/execution/thread-pool.h"
#include "infrastructure/http/router.h"
#include "infrastructure/security/auth-token-store.h"

namespace presentation::http {

class GroupController {

        using RouteContext = infrastructure::http::RouteContext;
        using Router = infrastructure::http::Router;
        using GroupService = domain::services::GroupService;
        using AuthTokenStore = infrastructure::security::AuthTokenStore;
        using ThreadPool = infrastructure::execution::ThreadPool;
        using NotificationPublisher = domain::notifications::NotificationPublisher;

        GroupService& m_groupService;
        infrastructure::security::AuthTokenStore& m_tokenStore;
        infrastructure::execution::ThreadPool& m_threadPool;
        NotificationPublisher* m_notificationPublisher{nullptr};

    public:

        GroupController(GroupService& groupService, AuthTokenStore& tokenStore, ThreadPool& threadPool) noexcept;

        GroupController(GroupService& groupService, AuthTokenStore& tokenStore, ThreadPool& threadPool,
                        NotificationPublisher& notificationPublisher) noexcept;

        void registerRoutes(Router& router);

    private:

        void handleCreateGroup(RouteContext& ctx);
        void handleDeleteGroup(RouteContext& ctx);
        void handleGetAllGroups(RouteContext& ctx);
        void handleGetGroupById(RouteContext& ctx);
        void handleAddUserToGroup(RouteContext& ctx);
        void handleRemoveUserFromGroup(RouteContext& ctx);
        void handleGetUserGroups(RouteContext& ctx);
        void handleGetGroupUsers(RouteContext& ctx);
};

}  // namespace presentation::http