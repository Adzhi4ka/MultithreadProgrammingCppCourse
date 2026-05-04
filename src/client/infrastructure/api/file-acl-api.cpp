#include "file-acl-api.h"

#include "json-utils.h"

#include <QJsonObject>
#include <QUrlQuery>

namespace client::infrastructure::api {

    FileAclApi::FileAclApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void FileAclApi::setGroupAcl(qint64 fileId,
                                 qint64 groupId,
                                 domain::models::AclLevel aclLevel,
                                 GroupAclCallback callback) {
        QJsonObject body{
            {"fileId", fileId},
            {"groupId", groupId},
            {"aclLevel", domain::models::toServerString(aclLevel)},
        };

        m_apiClient.putJson("/api/file-acl/groups", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseGroupAclResponse(response));
        });
    }

    void FileAclApi::removeGroupAcl(qint64 fileId, qint64 groupId, VoidCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));
        query.addQueryItem("groupId", QString::number(groupId));

        m_apiClient.deleteRequest("/api/file-acl/groups", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to remove ACL")));
                return;
            }

            callback(apiSuccess());
        });
    }

    void FileAclApi::getGroupAcl(qint64 fileId, qint64 groupId, GroupAclCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));
        query.addQueryItem("groupId", QString::number(groupId));

        m_apiClient.get("/api/file-acl/groups", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseGroupAclResponse(response));
        });
    }

    void FileAclApi::getUserAcl(qint64 fileId, qint64 userId, UserAclCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));
        query.addQueryItem("userId", QString::number(userId));

        m_apiClient.get("/api/file-acl/users", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseUserAclResponse(response));
        });
    }

    void FileAclApi::getByFile(qint64 fileId, GroupAclListCallback callback) {
        QUrlQuery query;
        query.addQueryItem("fileId", QString::number(fileId));

        m_apiClient.get("/api/file-acl/by-file", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseAclListResponse(response));
        });
    }

    void FileAclApi::getByGroup(qint64 groupId, GroupAclListCallback callback) {
        QUrlQuery query;
        query.addQueryItem("groupId", QString::number(groupId));

        m_apiClient.get("/api/file-acl/by-group", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseAclListResponse(response));
        });
    }

    FileAclApi::GroupAclResult FileAclApi::parseGroupAclResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "ACL operation failed"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseFileAcl(*object));
    }

    FileAclApi::UserAclResult FileAclApi::parseUserAclResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "failed to load user ACL"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseUserFileAcl(*object));
    }

    FileAclApi::GroupAclListResult FileAclApi::parseAclListResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "failed to load ACL list"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseFileAclItems(*object));
    }

}
