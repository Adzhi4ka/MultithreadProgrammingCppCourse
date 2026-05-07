#include <algorithm>

#include "service-test-integration.h"

namespace tests {

TEST_F(ServiceIntegrationTest, CreateGroup_ReturnsGroupId_WhenNameIsUnique) {
    auto createResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(createResult.has_value());
    EXPECT_GT(*createResult, 0);
}

TEST_F(ServiceIntegrationTest, CreateGroup_ReturnsConflict_WhenNameAlreadyExists) {
    auto firstCreateResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(firstCreateResult.has_value());

    auto secondCreateResult = m_groupService->createGroup("admins");
    ASSERT_FALSE(secondCreateResult.has_value());
    EXPECT_EQ(secondCreateResult.error(), ServiceError::Conflict);
}

TEST_F(ServiceIntegrationTest, AddUserToGroup_AddsMembership) {
    auto addUserResult = m_userService->addUser("ivan", "123456");
    ASSERT_TRUE(addUserResult.has_value());

    auto createGroupResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(createGroupResult.has_value());

    auto addMembershipResult = m_groupService->addUserToGroup(*addUserResult, *createGroupResult);
    ASSERT_TRUE(addMembershipResult.has_value());

    auto userGroupsResult = m_groupService->getUserGroups(*addUserResult);
    ASSERT_TRUE(userGroupsResult.has_value());
    ASSERT_EQ(userGroupsResult->size(), 2u);
    EXPECT_TRUE((*userGroupsResult)[0] == *createGroupResult || (*userGroupsResult)[1] == *createGroupResult);
}

TEST_F(ServiceIntegrationTest, AddUserToGroup_ReturnsConflict_WhenMembershipAlreadyExists) {
    auto addUserResult = m_userService->addUser("ivan", "123456");
    ASSERT_TRUE(addUserResult.has_value());

    auto createGroupResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(createGroupResult.has_value());

    auto firstAddMembershipResult = m_groupService->addUserToGroup(*addUserResult, *createGroupResult);
    ASSERT_TRUE(firstAddMembershipResult.has_value());

    auto secondAddMembershipResult = m_groupService->addUserToGroup(*addUserResult, *createGroupResult);
    ASSERT_FALSE(secondAddMembershipResult.has_value());
    EXPECT_EQ(secondAddMembershipResult.error(), ServiceError::Conflict);
}

TEST_F(ServiceIntegrationTest, RemoveUserFromGroup_RemovesMembership) {
    auto addUserResult = m_userService->addUser("ivan", "123456");
    ASSERT_TRUE(addUserResult.has_value());

    auto createGroupResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(createGroupResult.has_value());

    auto addMembershipResult = m_groupService->addUserToGroup(*addUserResult, *createGroupResult);
    ASSERT_TRUE(addMembershipResult.has_value());

    auto removeMembershipResult = m_groupService->removeUserFromGroup(*addUserResult, *createGroupResult);
    ASSERT_TRUE(removeMembershipResult.has_value());

    auto userGroupsResult = m_groupService->getUserGroups(*addUserResult);
    ASSERT_TRUE(userGroupsResult.has_value());
    ASSERT_EQ(userGroupsResult->size(), 1u);
}

TEST_F(ServiceIntegrationTest, RemoveUserFromGroup_ReturnsNotFound_WhenMembershipDoesNotExist) {
    auto addUserResult = m_userService->addUser("ivan", "123456");
    ASSERT_TRUE(addUserResult.has_value());

    auto createGroupResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(createGroupResult.has_value());

    auto removeMembershipResult = m_groupService->removeUserFromGroup(*addUserResult, *createGroupResult);
    ASSERT_FALSE(removeMembershipResult.has_value());
    EXPECT_EQ(removeMembershipResult.error(), ServiceError::NotFound);
}

TEST_F(ServiceIntegrationTest, GetUserGroups_ReturnsAllGroupsOfUser) {
    auto addUserResult = m_userService->addUser("ivan", "123456");
    ASSERT_TRUE(addUserResult.has_value());

    auto firstGroupResult = m_groupService->createGroup("admins");
    ASSERT_TRUE(firstGroupResult.has_value());

    auto secondGroupResult = m_groupService->createGroup("devs");
    ASSERT_TRUE(secondGroupResult.has_value());

    auto addFirstMembershipResult = m_groupService->addUserToGroup(*addUserResult, *firstGroupResult);
    ASSERT_TRUE(addFirstMembershipResult.has_value());

    auto addSecondMembershipResult = m_groupService->addUserToGroup(*addUserResult, *secondGroupResult);
    ASSERT_TRUE(addSecondMembershipResult.has_value());

    auto userGroupsResult = m_groupService->getUserGroups(*addUserResult);
    ASSERT_TRUE(userGroupsResult.has_value());
    ASSERT_EQ(userGroupsResult->size(), 3u);

    EXPECT_TRUE(std::ranges::find(*userGroupsResult, *firstGroupResult) != userGroupsResult->end());
    EXPECT_TRUE(std::ranges::find(*userGroupsResult, *secondGroupResult) != userGroupsResult->end());
}

}  // namespace tests