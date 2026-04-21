#include "service-test-integration.h"

namespace tests {

    TEST_F(ServiceIntegrationTest, CreateNewVersion_CreatesSecondVersion) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file.txt", 222);
        ASSERT_TRUE(createVersionResult.has_value());
        EXPECT_GT(*createVersionResult, 0);

        auto getAllVersionsResult = m_fileVersionService->getAllVersions(*createFileResult);
        ASSERT_TRUE(getAllVersionsResult.has_value());
        EXPECT_EQ(getAllVersionsResult->size(), 2u);
    }

    TEST_F(ServiceIntegrationTest, CreateNewVersion_UpdatesCurrentVersion) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto createVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v2.txt", 222);
        ASSERT_TRUE(createVersionResult.has_value());

        auto currentVersionResult = m_fileVersionService->getCurrentVersion(*createFileResult);
        ASSERT_TRUE(currentVersionResult.has_value());
        EXPECT_EQ(currentVersionResult->id, *createVersionResult);
        EXPECT_EQ(currentVersionResult->logicalNameSnapshot, "file_v2.txt");
        EXPECT_EQ(currentVersionResult->physicalPathName, 222u);
    }

    TEST_F(ServiceIntegrationTest, CreateNewVersion_IncrementsVersionNumber) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto secondVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v2.txt", 222);
        ASSERT_TRUE(secondVersionResult.has_value());

        auto thirdVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v3.txt", 333);
        ASSERT_TRUE(thirdVersionResult.has_value());

        auto getAllVersionsResult = m_fileVersionService->getAllVersions(*createFileResult);
        ASSERT_TRUE(getAllVersionsResult.has_value());
        ASSERT_EQ(getAllVersionsResult->size(), 3u);

        EXPECT_EQ((*getAllVersionsResult)[0].version, 1u);
        EXPECT_EQ((*getAllVersionsResult)[1].version, 2u);
        EXPECT_EQ((*getAllVersionsResult)[2].version, 3u);
    }

    TEST_F(ServiceIntegrationTest, CreateNewVersion_ReturnsNotFound_WhenFileDoesNotExist) {
        auto createVersionResult =
            m_fileVersionService->createNewVersion(999999, "ghost.txt", 111);
        ASSERT_FALSE(createVersionResult.has_value());
        EXPECT_EQ(createVersionResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, GetCurrentVersion_ReturnsFirstVersion_AfterCreate) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto currentVersionResult = m_fileVersionService->getCurrentVersion(*createFileResult);
        ASSERT_TRUE(currentVersionResult.has_value());
        EXPECT_EQ(currentVersionResult->version, 1u);
        EXPECT_EQ(currentVersionResult->logicalNameSnapshot, "file.txt");
        EXPECT_EQ(currentVersionResult->physicalPathName, 111u);
    }

    TEST_F(ServiceIntegrationTest, GetCurrentVersion_ReturnsLatestVersion_WhenSeveralVersionsExist) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto secondVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v2.txt", 222);
        ASSERT_TRUE(secondVersionResult.has_value());

        auto thirdVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v3.txt", 333);
        ASSERT_TRUE(thirdVersionResult.has_value());

        auto currentVersionResult = m_fileVersionService->getCurrentVersion(*createFileResult);
        ASSERT_TRUE(currentVersionResult.has_value());
        EXPECT_EQ(currentVersionResult->id, *thirdVersionResult);
        EXPECT_EQ(currentVersionResult->version, 3u);
        EXPECT_EQ(currentVersionResult->logicalNameSnapshot, "file_v3.txt");
        EXPECT_EQ(currentVersionResult->physicalPathName, 333u);
    }

    TEST_F(ServiceIntegrationTest, GetCurrentVersion_ReturnsNotFound_WhenFileDoesNotExist) {
        auto currentVersionResult = m_fileVersionService->getCurrentVersion(999999);
        ASSERT_FALSE(currentVersionResult.has_value());
        EXPECT_EQ(currentVersionResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, GetAllVersions_ReturnsAllVersionsInOrder) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto secondVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v2.txt", 222);
        ASSERT_TRUE(secondVersionResult.has_value());

        auto thirdVersionResult =
            m_fileVersionService->createNewVersion(*createFileResult, "file_v3.txt", 333);
        ASSERT_TRUE(thirdVersionResult.has_value());

        auto getAllVersionsResult = m_fileVersionService->getAllVersions(*createFileResult);
        ASSERT_TRUE(getAllVersionsResult.has_value());
        ASSERT_EQ(getAllVersionsResult->size(), 3u);

        EXPECT_EQ((*getAllVersionsResult)[0].version, 1u);
        EXPECT_EQ((*getAllVersionsResult)[0].logicalNameSnapshot, "file.txt");
        EXPECT_EQ((*getAllVersionsResult)[0].physicalPathName, 111u);

        EXPECT_EQ((*getAllVersionsResult)[1].version, 2u);
        EXPECT_EQ((*getAllVersionsResult)[1].logicalNameSnapshot, "file_v2.txt");
        EXPECT_EQ((*getAllVersionsResult)[1].physicalPathName, 222u);

        EXPECT_EQ((*getAllVersionsResult)[2].version, 3u);
        EXPECT_EQ((*getAllVersionsResult)[2].logicalNameSnapshot, "file_v3.txt");
        EXPECT_EQ((*getAllVersionsResult)[2].physicalPathName, 333u);
    }

    TEST_F(ServiceIntegrationTest, GetAllVersions_ReturnsOneVersion_AfterCreate) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto getAllVersionsResult = m_fileVersionService->getAllVersions(*createFileResult);
        ASSERT_TRUE(getAllVersionsResult.has_value());
        ASSERT_EQ(getAllVersionsResult->size(), 1u);

        EXPECT_EQ((*getAllVersionsResult)[0].version, 1u);
        EXPECT_EQ((*getAllVersionsResult)[0].logicalNameSnapshot, "file.txt");
        EXPECT_EQ((*getAllVersionsResult)[0].physicalPathName, 111u);
    }

    TEST_F(ServiceIntegrationTest, GetAllVersions_ReturnsNotFound_WhenFileDoesNotExist) {
        auto getAllVersionsResult = m_fileVersionService->getAllVersions(999999);
        ASSERT_FALSE(getAllVersionsResult.has_value());
        EXPECT_EQ(getAllVersionsResult.error(), ServiceError::NotFound);
    }

} // namespace tests