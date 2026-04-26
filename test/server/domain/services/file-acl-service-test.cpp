#include "service-test-integration.h"

#include "domain/models/file-acl.h"
#include "gtest/gtest.h"

namespace tests {

    TEST_F(ServiceIntegrationTest, CreateGroupAclLevel_CreatesReadOnlyAcl) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto createAclResult =
            m_fileAclService->createGroupAclLevel(*createFileResult,
                                                  *createGroupResult,
                                                  domain::models::AclLevel::READ_ONLY);
        ASSERT_TRUE(createAclResult.has_value());

        auto getAclResult =
            m_fileAclService->getGroupAclLevel(*createGroupResult, *createFileResult);
        ASSERT_TRUE(getAclResult.has_value());
        EXPECT_EQ(*getAclResult, domain::models::AclLevel::READ_ONLY);
    }

    TEST_F(ServiceIntegrationTest, CreateGroupAclLevel_CreatesReadWriteAcl) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto createAclResult =
            m_fileAclService->createGroupAclLevel(*createFileResult,
                                                  *createGroupResult,
                                                  domain::models::AclLevel::READ_WRITE);
        ASSERT_TRUE(createAclResult.has_value());

        auto getAclResult =
            m_fileAclService->getGroupAclLevel(*createGroupResult, *createFileResult);
        ASSERT_TRUE(getAclResult.has_value());
        EXPECT_EQ(*getAclResult, domain::models::AclLevel::READ_WRITE);
    }

    TEST_F(ServiceIntegrationTest, CreateGroupAclLevel_ReturnsConflict_WhenAclAlreadyExists) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto firstCreateAclResult =
            m_fileAclService->createGroupAclLevel(*createFileResult,
                                                  *createGroupResult,
                                                  domain::models::AclLevel::READ_ONLY);
        ASSERT_TRUE(firstCreateAclResult.has_value());

        auto secondCreateAclResult =
            m_fileAclService->createGroupAclLevel(*createFileResult,
                                                  *createGroupResult,
                                                  domain::models::AclLevel::READ_WRITE);
        ASSERT_FALSE(secondCreateAclResult.has_value());
        EXPECT_EQ(secondCreateAclResult.error(), ServiceError::Conflict);
    }

    TEST_F(ServiceIntegrationTest, GetGroupAclLevel_ReturnsNoProperty_WhenAclDoesNotExist) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto getAclResult =
            m_fileAclService->getGroupAclLevel(*createGroupResult, *createFileResult);
        ASSERT_TRUE(getAclResult.has_value());
        EXPECT_EQ(*getAclResult, domain::models::AclLevel::NO_PROPERTY);
    }

    TEST_F(ServiceIntegrationTest, RemoveGroupAclLevel_RemovesAcl) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto createAclResult =
            m_fileAclService->createGroupAclLevel(*createFileResult,
                                                  *createGroupResult,
                                                  domain::models::AclLevel::READ_ONLY);
        ASSERT_TRUE(createAclResult.has_value());

        auto getAclResult =
            m_fileAclService->getGroupAclLevel(*createGroupResult, *createFileResult);
        ASSERT_TRUE(getAclResult.has_value());
        EXPECT_EQ(*getAclResult, domain::models::AclLevel::READ_ONLY);

        auto removeAclResult =
            m_fileAclService->removeGroupAclLevel(*createFileResult, *createGroupResult);
        ASSERT_TRUE(removeAclResult.has_value());

        getAclResult =
            m_fileAclService->getGroupAclLevel(*createGroupResult, *createFileResult);
        ASSERT_TRUE(getAclResult.has_value());
        EXPECT_EQ(*getAclResult, domain::models::AclLevel::NO_PROPERTY);
    }

    TEST_F(ServiceIntegrationTest, RemoveGroupAclLevel_ReturnsNotFound_WhenAclDoesNotExist) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto removeAclResult =
            m_fileAclService->removeGroupAclLevel(*createFileResult, *createGroupResult);
        ASSERT_FALSE(removeAclResult.has_value());
        EXPECT_EQ(removeAclResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, GetUserAclLevel_ReturnsGroupAcl_WhenUserBelongsToGroup) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto addMembershipResult =
            m_groupService->addUserToGroup(*addUserResult, *createGroupResult);
        ASSERT_TRUE(addMembershipResult.has_value());

        auto createAclResult =
            m_fileAclService->createGroupAclLevel(*createFileResult,
                                                  *createGroupResult,
                                                  domain::models::AclLevel::READ_ONLY);
        ASSERT_TRUE(createAclResult.has_value());

        auto getUserAclResult =
            m_fileAclService->getUserAclLevel(*addUserResult, *createFileResult);
        ASSERT_TRUE(getUserAclResult.has_value());
        EXPECT_EQ(*getUserAclResult, domain::models::AclLevel::READ_ONLY);
    }

    TEST_F(ServiceIntegrationTest, GetUserAclLevel_ReturnsNoProperty_WhenUserHasNoGroups) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto getUserAclResult =
            m_fileAclService->getUserAclLevel(*addUserResult, *createFileResult);
        ASSERT_TRUE(getUserAclResult.has_value());
        EXPECT_EQ(*getUserAclResult,  domain::models::AclLevel::NO_PROPERTY);
    }

    TEST_F(ServiceIntegrationTest, GetUserAclLevel_ReturnsNoProperty_WhenUserGroupsHaveNoAclForFile) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createGroupResult = m_groupService->createGroup("admins");
        ASSERT_TRUE(createGroupResult.has_value());

        auto addMembershipResult =
            m_groupService->addUserToGroup(*addUserResult, *createGroupResult);
        ASSERT_TRUE(addMembershipResult.has_value());

        auto getUserAclResult =
            m_fileAclService->getUserAclLevel(*addUserResult, *createFileResult);
        ASSERT_TRUE(getUserAclResult.has_value());
        EXPECT_EQ(*getUserAclResult,  domain::models::AclLevel::NO_PROPERTY);
    }

} // namespace tests