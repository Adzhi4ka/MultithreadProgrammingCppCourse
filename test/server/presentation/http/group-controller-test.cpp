#include "controller-test-integration.h"

namespace tests {

    TEST_F(ControllerIntegrationTest, GroupController_CreateMembershipAndQueries_Work) {
        auto alice = registerUser("alice", "alice123");
        auto bob = registerUser("bob", "bob123");

        const auto groupId = createGroup(alice.token, "admins");
        addUserToGroup(alice.token, bob.userId, groupId);

        auto getAllResponse = request(http::verb::get, "/api/groups", {}, alice.token);
        ASSERT_EQ(getAllResponse.result(), http::status::ok) << getAllResponse.body();

        auto allGroups = parseJson(getAllResponse).as_object();
        ASSERT_TRUE(allGroups.if_contains("items"));
        ASSERT_TRUE(allGroups.at("items").is_array());
        EXPECT_EQ(allGroups.at("items").as_array().size(), 3u);

        auto getByIdResponse = request(http::verb::get,
                                       "/api/groups/by-id?groupId=" + std::to_string(groupId),
                                       {},
                                       alice.token);
        ASSERT_EQ(getByIdResponse.result(), http::status::ok) << getByIdResponse.body();

        auto groupById = parseJson(getByIdResponse).as_object();
        EXPECT_EQ(groupById.at("id").as_int64(), groupId);
        EXPECT_EQ(groupById.at("name").as_string(), "admins");

        auto userGroupsResponse = request(http::verb::get,
                                          "/api/groups/by-user?userId=" + std::to_string(bob.userId),
                                          {},
                                          alice.token);
        ASSERT_EQ(userGroupsResponse.result(), http::status::ok) << userGroupsResponse.body();

        auto userGroups = parseJson(userGroupsResponse).as_object().at("items").as_array();
        ASSERT_EQ(userGroups.size(), 2u);
        const auto firstUserGroupId = userGroups[0].as_object().at("groupId").as_int64();
        const auto secondUserGroupId = userGroups[1].as_object().at("groupId").as_int64();
        EXPECT_TRUE(firstUserGroupId == groupId || secondUserGroupId == groupId);

        auto groupUsersResponse = request(http::verb::get,
                                          "/api/groups/users?groupId=" + std::to_string(groupId),
                                          {},
                                          alice.token);
        ASSERT_EQ(groupUsersResponse.result(), http::status::ok) << groupUsersResponse.body();

        auto groupUsers = parseJson(groupUsersResponse).as_object().at("items").as_array();
        ASSERT_EQ(groupUsers.size(), 1u);
        EXPECT_EQ(groupUsers[0].as_object().at("userId").as_int64(), bob.userId);
    }

} // namespace tests