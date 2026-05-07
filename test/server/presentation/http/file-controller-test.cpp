#include "controller-test-integration.h"

namespace tests {

TEST_F(ControllerIntegrationTest, FileController_RespectsAclForReadAndWriteOperations) {
    auto alice = registerUser("alice", "alice123");
    auto bob = registerUser("bob", "bob123");

    const auto sharedGroupId = createGroup(alice.token, "shared");
    addUserToGroup(alice.token, alice.userId, sharedGroupId);
    addUserToGroup(alice.token, bob.userId, sharedGroupId);

    const auto fileId = createFile(alice.token, "docs/report.txt");

    auto forbiddenGetResponse =
        request(http::verb::get, "/api/files/by-id?fileId=" + std::to_string(fileId), {}, bob.token);
    EXPECT_EQ(forbiddenGetResponse.result(), http::status::forbidden);

    setGroupAcl(alice.token, fileId, sharedGroupId, "read_only");

    auto allowedGetResponse =
        request(http::verb::get, "/api/files/by-id?fileId=" + std::to_string(fileId), {}, bob.token);
    ASSERT_EQ(allowedGetResponse.result(), http::status::ok) << allowedGetResponse.body();

    auto fileJson = parseJson(allowedGetResponse).as_object();
    EXPECT_EQ(fileJson.at("id").as_int64(), fileId);
    EXPECT_EQ(fileJson.at("fullLogicalName").as_string(), "docs/report.txt");

    json::object renameBody{{"fileId", fileId}, {"newLogicalName", "docs/report-renamed.txt"}};

    auto forbiddenRenameResponse =
        request(http::verb::put, "/api/files/rename", json::serialize(renameBody), bob.token);
    EXPECT_EQ(forbiddenRenameResponse.result(), http::status::forbidden);

    setGroupAcl(alice.token, fileId, sharedGroupId, "read_write");

    auto allowedRenameResponse = request(http::verb::put, "/api/files/rename", json::serialize(renameBody), bob.token);
    ASSERT_EQ(allowedRenameResponse.result(), http::status::ok) << allowedRenameResponse.body();

    auto renamedJson = parseJson(allowedRenameResponse).as_object();
    EXPECT_EQ(renamedJson.at("fullLogicalName").as_string(), "docs/report-renamed.txt");

    auto getAllResponse = request(http::verb::get, "/api/files", {}, bob.token);
    ASSERT_EQ(getAllResponse.result(), http::status::ok) << getAllResponse.body();

    auto items = parseJson(getAllResponse).as_object().at("items").as_array();
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].as_object().at("id").as_int64(), fileId);

    auto deleteResponse = request(http::verb::delete_, "/api/files?fileId=" + std::to_string(fileId), {}, bob.token);
    EXPECT_EQ(deleteResponse.result(), http::status::ok);
}

TEST_F(ControllerIntegrationTest, FileAclController_SetGetEffectiveAndList_Work) {
    auto alice = registerUser("alice", "alice123");
    auto bob = registerUser("bob", "bob123");

    const auto readersGroupId = createGroup(alice.token, "readers");
    addUserToGroup(alice.token, bob.userId, readersGroupId);

    const auto fileId = createFile(alice.token, "docs/public.txt");

    setGroupAcl(alice.token, fileId, readersGroupId, "read_write");

    auto groupAclResponse =
        request(http::verb::get,
                "/api/file-acl/groups?fileId=" + std::to_string(fileId) + "&groupId=" + std::to_string(readersGroupId),
                {}, alice.token);
    ASSERT_EQ(groupAclResponse.result(), http::status::ok) << groupAclResponse.body();

    auto groupAclJson = parseJson(groupAclResponse).as_object();
    EXPECT_EQ(groupAclJson.at("aclLevel").as_string(), "read_write");

    auto userAclResponse =
        request(http::verb::get,
                "/api/file-acl/users?fileId=" + std::to_string(fileId) + "&userId=" + std::to_string(bob.userId), {},
                alice.token);
    ASSERT_EQ(userAclResponse.result(), http::status::ok) << userAclResponse.body();

    auto userAclJson = parseJson(userAclResponse).as_object();
    EXPECT_EQ(userAclJson.at("aclLevel").as_string(), "read_write");

    auto byFileResponse =
        request(http::verb::get, "/api/file-acl/by-file?fileId=" + std::to_string(fileId), {}, alice.token);
    ASSERT_EQ(byFileResponse.result(), http::status::ok) << byFileResponse.body();

    auto byFileItems = parseJson(byFileResponse).as_object().at("items").as_array();
    ASSERT_EQ(byFileItems.size(), 2u);

    auto byGroupResponse =
        request(http::verb::get, "/api/file-acl/by-group?groupId=" + std::to_string(readersGroupId), {}, alice.token);
    ASSERT_EQ(byGroupResponse.result(), http::status::ok) << byGroupResponse.body();

    auto byGroupItems = parseJson(byGroupResponse).as_object().at("items").as_array();
    ASSERT_EQ(byGroupItems.size(), 1u);
}

}  // namespace tests