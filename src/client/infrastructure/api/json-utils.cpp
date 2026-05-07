#include "json-utils.h"
#include "api-client.h"
#include "qtypes.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>

#include <utility>

namespace client::infrastructure::api {

    namespace {

        std::optional<QJsonValue> getRequiredValue(const QJsonObject& object, QStringView fieldName) {
            const auto key = fieldName.toString();
            const auto it = object.find(key);
            if (it == object.end() || it->isUndefined() || it->isNull()) {
                return std::nullopt;
            }

            return *it;
        }

        std::optional<qint64> numberToInt64(const QJsonValue& value) {
            if (!value.isDouble()) {
                return std::nullopt;
            }

            return (qint64)value.toDouble();
        }

    }

    std::optional<QJsonObject> parseJsonObject(const QByteArray& bytes, QString* errorMessage) {
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(bytes, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            if (errorMessage) {
                *errorMessage = parseError.errorString();
            }
            return std::nullopt;
        }

        if (!document.isObject()) {
            if (errorMessage) {
                *errorMessage = "response is not a JSON object";
            }
            return std::nullopt;
        }

        return document.object();
    }

    std::optional<QJsonArray> getArrayField(const QJsonObject& object, QStringView fieldName) {
        const auto value = getRequiredValue(object, fieldName);
        if (!value || !value->isArray()) {
            return std::nullopt;
        }

        return value->toArray();
    }

    std::optional<qint64> getInt64Field(const QJsonObject& object, QStringView fieldName) {
        const auto value = getRequiredValue(object, fieldName);
        if (!value) {
            return std::nullopt;
        }

        return numberToInt64(*value);
    }

    std::optional<quint32> getUInt32Field(const QJsonObject& object, QStringView fieldName) {
        const auto value = getInt64Field(object, fieldName);
        if (!value || *value < 0) {
            return std::nullopt;
        }

        return *value;
    }

    std::optional<QString> getStringField(const QJsonObject& object, QStringView fieldName) {
        const auto value = getRequiredValue(object, fieldName);
        if (!value || !value->isString()) {
            return std::nullopt;
        }

        return value->toString();
    }

    ApiError makeHttpError(const RawApiResponse& response, QString fallbackMessage) {
        ApiError error;
        error.httpStatus = response.httpStatus;
        error.message = std::move(fallbackMessage);

        QString parseError;
        if (const auto object = parseJsonObject(response.body, &parseError)) {
            if (const auto serverError = getStringField(*object, u"error")) {
                error.code = *serverError;
                error.message = *serverError;
            }
        } else if (!response.errorText.isEmpty()) {
            error.message = response.errorText;
        }

        return error;
    }

    domain::models::UserSession parseUserSession(const QJsonObject& object) {
        return domain::models::UserSession{
            .userId = getInt64Field(object, u"userId").value_or(0),
            .login = getStringField(object, u"login").value_or(QString{}),
            .token = getStringField(object, u"token").value_or(QString{}),
        };
    }

    domain::models::UserProfile parseUserProfile(const QJsonObject& object) {
        return domain::models::UserProfile{
            .userId = getInt64Field(object, u"userId").value_or(getInt64Field(object, u"id").value_or(0)),
            .login = getStringField(object, u"login").value_or(QString{}),
        };
    }

    domain::models::RemoteFile parseRemoteFile(const QJsonObject& object) {
        return domain::models::RemoteFile{
            .id = getInt64Field(object, u"id").value_or(0),
            .fullLogicalName = getStringField(object, u"fullLogicalName").value_or(QString{}),
            .currentVersionId = getInt64Field(object, u"currentVersionId").value_or(0),
            .maxVersionCount = getUInt32Field(object, u"maxVersionCount").value_or(0),
            .createdAt = getInt64Field(object, u"createdAt").value_or(0),
            .createdBy = getInt64Field(object, u"createdBy").value_or(0),
        };
    }

    domain::models::FileVersion parseFileVersion(const QJsonObject& object) {
        return domain::models::FileVersion{
            .id = getInt64Field(object, u"id").value_or(0),
            .fileId = getInt64Field(object, u"fileId").value_or(0),
            .version = (qint32)getInt64Field(object, u"version").value_or(0),
            .logicalNameSnapshot = getStringField(object, u"logicalNameSnapshot").value_or(QString{}),
            .createdAt = getInt64Field(object, u"createdAt").value_or(0),
        };
    }

    domain::models::FileLock parseFileLock(const QJsonObject& object) {
        return domain::models::FileLock{
            .fileId = getInt64Field(object, u"fileId").value_or(0),
            .userId = getInt64Field(object, u"userId").value_or(0),
            .leaseUntil = getInt64Field(object, u"leaseUntil").value_or(0),
            .lockToken = getInt64Field(object, u"lockToken").value_or(0),
        };
    }

    domain::models::FileAcl parseFileAcl(const QJsonObject& object) {
        const auto aclText = getStringField(object, u"aclLevel").value_or(QString{});

        return domain::models::FileAcl{
            .fileId = getInt64Field(object, u"fileId").value_or(0),
            .groupId = getInt64Field(object, u"groupId").value_or(0),
            .aclLevel = domain::models::aclLevelFromServerString(aclText),
        };
    }

    domain::models::UserFileAcl parseUserFileAcl(const QJsonObject& object) {
        const auto aclText = getStringField(object, u"aclLevel").value_or(QString{});

        return domain::models::UserFileAcl{
            .fileId = getInt64Field(object, u"fileId").value_or(0),
            .userId = getInt64Field(object, u"userId").value_or(0),
            .aclLevel = domain::models::aclLevelFromServerString(aclText),
        };
    }

    domain::models::Group parseGroup(const QJsonObject& object) {
        return domain::models::Group{
            .id = getInt64Field(object, u"id").value_or(0),
            .name = getStringField(object, u"name").value_or(QString{}),
        };
    }

    std::vector<domain::models::RemoteFile> parseRemoteFileItems(const QJsonObject& root) {
        std::vector<domain::models::RemoteFile> result;

        const auto items = getArrayField(root, u"items");
        if (!items) {
            return result;
        }

        result.reserve(items->size());
        for (const auto& item : *items) {
            if (item.isObject()) {
                result.emplace_back(parseRemoteFile(item.toObject()));
            }
        }

        return result;
    }

    std::vector<domain::models::FileVersion> parseFileVersionItems(const QJsonObject& root) {
        std::vector<domain::models::FileVersion> result;

        const auto items = getArrayField(root, u"items");
        if (!items) {
            return result;
        }

        result.reserve(items->size());
        for (const auto& item : *items) {
            if (item.isObject()) {
                result.emplace_back(parseFileVersion(item.toObject()));
            }
        }

        return result;
    }

    std::vector<domain::models::FileAcl> parseFileAclItems(const QJsonObject& root) {
        std::vector<domain::models::FileAcl> result;

        const auto items = getArrayField(root, u"items");
        if (!items) {
            return result;
        }

        result.reserve(items->size());
        for (const auto& item : *items) {
            if (item.isObject()) {
                result.emplace_back(parseFileAcl(item.toObject()));
            }
        }

        return result;
    }

    std::vector<domain::models::Group> parseGroupItems(const QJsonObject& root) {
        std::vector<domain::models::Group> result;

        const auto items = getArrayField(root, u"items");
        if (!items) {
            return result;
        }

        result.reserve(items->size());
        for (const auto& item : *items) {
            if (item.isObject()) {
                result.emplace_back(parseGroup(item.toObject()));
            }
        }

        return result;
    }

}
