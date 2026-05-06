#pragma once

#include "api-client.h"
#include "api-result.h"
#include "domain/models/acl-level.h"
#include "domain/models/file-acl.h"

#include <QObject>

#include <functional>
#include <vector>

namespace client::infrastructure::api {

    class FileAclApi : public QObject {

            using FileAcl = domain::models::FileAcl;
            using UserFileAcl = domain::models::UserFileAcl;
            using AclLevel = domain::models::AclLevel;

            Q_OBJECT
            ApiClient& m_apiClient;
    
        public:

            explicit FileAclApi(ApiClient& apiClient, QObject* parent = nullptr);
    
            void setGroupAcl(qint64 fileId,
                             qint64 groupId,
                             AclLevel aclLevel,
                             std::function<void(ApiResult<FileAcl>)> callback);
            void removeGroupAcl(qint64 fileId,
                                qint64 groupId,
                                std::function<void(ApiResult<void>)> callback);
            void getGroupAcl(qint64 fileId,
                             qint64 groupId,
                             std::function<void(ApiResult<FileAcl>)> callback);
            void getUserAcl(qint64 fileId,
                            qint64 userId,
                            std::function<void(ApiResult<UserFileAcl>)> callback);
            void getByFile(qint64 fileId,
                           std::function<void(ApiResult<std::vector<FileAcl>>)> callback);
            void getByGroup(qint64 groupId,
                            std::function<void(ApiResult<std::vector<FileAcl>>)> callback);
    
        private:
        
            static ApiResult<FileAcl> parseGroupAclResponse(const RawApiResponse& response);
            static ApiResult<UserFileAcl> parseUserAclResponse(const RawApiResponse& response);
            static ApiResult<std::vector<FileAcl>> parseAclListResponse(const RawApiResponse& response);

    };

}
