#include "controller-test-integration.h"

namespace tests {

TEST_F(ControllerIntegrationTest, FileVersionController_WorksWithTwoLargePhysicalFiles) {
    auto alice = registerUser("alice", "alice123");
    auto bob = registerUser("bob", "bob123");

    const auto editorsGroupId = createGroup(alice.token, "editors");
    addUserToGroup(alice.token, alice.userId, editorsGroupId);
    addUserToGroup(alice.token, bob.userId, editorsGroupId);

    const auto fileId = createFile(alice.token, "media/big.bin", 10);
    setGroupAcl(alice.token, fileId, editorsGroupId, "read_write");

    const auto payloadV1 = makeLargePayload(4 * 1024 * 1024, 17);
    writeCurrentVersionContent(fileId, payloadV1);

    auto currentV1Response =
        request(http::verb::get, "/api/file-versions/current?fileId=" + std::to_string(fileId), {}, bob.token);
    ASSERT_EQ(currentV1Response.result(), http::status::ok) << currentV1Response.body();

    auto currentV1Json = parseJson(currentV1Response).as_object();
    EXPECT_EQ(currentV1Json.at("version").as_int64(), 1);

    const auto version2Id = createVersion(bob.token, fileId, "media/big-v2.bin");
    EXPECT_GT(version2Id, 0);

    const auto payloadV2 = makeLargePayload(9 * 1024 * 1024, 83);
    writeCurrentVersionContent(fileId, payloadV2);

    auto currentV2Response =
        request(http::verb::get, "/api/file-versions/current?fileId=" + std::to_string(fileId), {}, bob.token);
    ASSERT_EQ(currentV2Response.result(), http::status::ok) << currentV2Response.body();

    auto currentV2Json = parseJson(currentV2Response).as_object();
    EXPECT_EQ(currentV2Json.at("id").as_int64(), version2Id);
    EXPECT_EQ(currentV2Json.at("version").as_int64(), 2);

    auto allVersionsResponse =
        request(http::verb::get, "/api/file-versions?fileId=" + std::to_string(fileId), {}, bob.token);
    ASSERT_EQ(allVersionsResponse.result(), http::status::ok) << allVersionsResponse.body();

    auto items = parseJson(allVersionsResponse).as_object().at("items").as_array();
    ASSERT_EQ(items.size(), 2u);

    EXPECT_EQ(items[0].as_object().at("version").as_int64(), 1);
    EXPECT_EQ(items[1].as_object().at("version").as_int64(), 2);
}

TEST_F(ControllerIntegrationTest, FileVersionController_RequiresWriteAclForNewVersion) {
    auto alice = registerUser("alice", "alice123");
    auto bob = registerUser("bob", "bob123");

    const auto readersGroupId = createGroup(alice.token, "readers");
    addUserToGroup(alice.token, bob.userId, readersGroupId);

    const auto fileId = createFile(alice.token, "docs/versioned.txt");
    setGroupAcl(alice.token, fileId, readersGroupId, "read_only");

    json::object body{{"fileId", fileId}, {"logicalNameSnapshot", "docs/versioned-v2.txt"}};

    auto forbiddenCreateVersionResponse =
        request(http::verb::post, "/api/file-versions", json::serialize(body), bob.token);
    EXPECT_EQ(forbiddenCreateVersionResponse.result(), http::status::forbidden);

    auto allowedReadResponse =
        request(http::verb::get, "/api/file-versions/current?fileId=" + std::to_string(fileId), {}, bob.token);
    EXPECT_EQ(allowedReadResponse.result(), http::status::ok);
}

}  // namespace tests