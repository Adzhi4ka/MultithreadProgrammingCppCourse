#include "group-api.h"

#include "json-utils.h"

#include <QJsonObject>
#include <QUrlQuery>

namespace client::infrastructure::api {

    GroupApi::GroupApi(ApiClient& apiClient, QObject* parent)
        : QObject(parent),
          m_apiClient(apiClient) {}

    void GroupApi::create(const QString& name, GroupCallback callback) {
        QJsonObject body{{"name", name}};

        m_apiClient.postJson("/api/groups", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseGroupResponse(response));
        });
    }

    void GroupApi::getAll(GroupsCallback callback) {
        m_apiClient.get("/api/groups", {}, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to load groups")));
                return;
            }

            QString parseError;
            const auto object = parseJsonObject(response.body, &parseError);
            if (!object) {
                callback(apiFailure(ApiError{response.httpStatus, "invalid_json", parseError}));
                return;
            }

            callback(apiSuccess(parseGroupItems(*object)));
        });
    }

    void GroupApi::getById(qint64 groupId, GroupCallback callback) {
        QUrlQuery query;
        query.addQueryItem("groupId", QString::number(groupId));

        m_apiClient.get("/api/groups/by-id", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseGroupResponse(response));
        });
    }

    void GroupApi::addUser(qint64 userId, qint64 groupId, VoidCallback callback) {
        QJsonObject body{
            {"userId", userId},
            {"groupId", groupId},
        };

        m_apiClient.postJson("/api/groups/members", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to add user to group")));
                return;
            }

            callback(apiSuccess());
        });
    }

    void GroupApi::removeUser(qint64 userId, qint64 groupId, VoidCallback callback) {
        QJsonObject body{
            {"userId", userId},
            {"groupId", groupId},
        };

        m_apiClient.deleteJson("/api/groups/members", body, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to remove user from group")));
                return;
            }

            callback(apiSuccess());
        });
    }

    void GroupApi::getUserGroups(qint64 userId, IdListCallback callback) {
        QUrlQuery query;
        query.addQueryItem("userId", QString::number(userId));

        m_apiClient.get("/api/groups/by-user", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseIdListResponse(response, "groupId"));
        });
    }

    void GroupApi::getGroupUsers(qint64 groupId, IdListCallback callback) {
        QUrlQuery query;
        query.addQueryItem("groupId", QString::number(groupId));

        m_apiClient.get("/api/groups/users", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            callback(parseIdListResponse(response, "userId"));
        });
    }

    void GroupApi::remove(qint64 groupId, VoidCallback callback) {
        QUrlQuery query;
        query.addQueryItem("groupId", QString::number(groupId));

        m_apiClient.deleteRequest("/api/groups", query, [callback = std::move(callback)](RawApiResponse response) mutable {
            if (!response.isSuccessStatus()) {
                callback(apiFailure(makeHttpError(response, "failed to remove group")));
                return;
            }

            callback(apiSuccess());
        });
    }

    GroupApi::IdListResult GroupApi::parseIdListResponse(const RawApiResponse& response, const QString& fieldName) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "failed to load ids"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        const auto items = getArrayField(*object, u"items");
        if (!items) {
            return apiFailure(ApiError{response.httpStatus, "invalid_items", "items field is missing"});
        }

        std::vector<qint64> result;
        result.reserve(items->size());
        for (const auto& item : *items) {
            if (!item.isObject()) {
                continue;
            }

            if (const auto id = getInt64Field(item.toObject(), fieldName)) {
                result.emplace_back(*id);
            }
        }

        return apiSuccess(std::move(result));
    }

    GroupApi::GroupResult GroupApi::parseGroupResponse(const RawApiResponse& response) {
        if (!response.isSuccessStatus()) {
            return apiFailure(makeHttpError(response, "group operation failed"));
        }

        QString parseError;
        const auto object = parseJsonObject(response.body, &parseError);
        if (!object) {
            return apiFailure(ApiError{response.httpStatus, "invalid_json", parseError});
        }

        return apiSuccess(parseGroup(*object));
    }

}
