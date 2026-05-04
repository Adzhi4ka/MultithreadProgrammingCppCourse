#pragma once

#include "api-client.h"
#include "api-result.h"

#include "domain/models/acl-level.h"
#include "domain/models/file-acl.h"

#include <QObject>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class FileAclApi final : public QObject {
        Q_OBJECT

    public:
        using GroupAclResult = ApiResult<domain::models::FileAcl>;
        using UserAclResult = ApiResult<domain::models::UserFileAcl>;
        using GroupAclListResult = ApiResult<std::vector<domain::models::FileAcl>>;
        using GroupAclCallback = std::function<void(GroupAclResult)>;
        using UserAclCallback = std::function<void(UserAclResult)>;
        using GroupAclListCallback = std::function<void(GroupAclListResult)>;
        using VoidCallback = std::function<void(ApiResult<void>)>;

        explicit FileAclApi(ApiClient& apiClient, QObject* parent = nullptr);

        void setGroupAcl(qint64 fileId,
                         qint64 groupId,
                         domain::models::AclLevel aclLevel,
                         GroupAclCallback callback);
        void removeGroupAcl(qint64 fileId, qint64 groupId, VoidCallback callback);
        void getGroupAcl(qint64 fileId, qint64 groupId, GroupAclCallback callback);
        void getUserAcl(qint64 fileId, qint64 userId, UserAclCallback callback);
        void getByFile(qint64 fileId, GroupAclListCallback callback);
        void getByGroup(qint64 groupId, GroupAclListCallback callback);

    private:
        static GroupAclResult parseGroupAclResponse(const RawApiResponse& response);
        static UserAclResult parseUserAclResponse(const RawApiResponse& response);
        static GroupAclListResult parseAclListResponse(const RawApiResponse& response);

    private:
        ApiClient& m_apiClient;
    };

}
