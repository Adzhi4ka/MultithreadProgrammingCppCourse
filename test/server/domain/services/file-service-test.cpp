#include "service-test-integration.h"

namespace tests {

    TEST_F(ServiceIntegrationTest, Create_CreatesFileAndReturnsFileId) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());
        EXPECT_GT(*createFileResult, 0);

        auto getByIdResult = m_fileService->getById(*createFileResult);
        ASSERT_TRUE(getByIdResult.has_value());
        EXPECT_EQ(getByIdResult->id, *createFileResult);
        EXPECT_EQ(getByIdResult->fullLogicalName, "file.txt");
        EXPECT_EQ(getByIdResult->createdBy, *addUserResult);
        EXPECT_EQ(getByIdResult->maxVersionCount, 10u);
    }

    TEST_F(ServiceIntegrationTest, Create_AllowsLookupByLogicalName) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto getByLogicalNameResult = m_fileService->getByLogicalName("file.txt");
        ASSERT_TRUE(getByLogicalNameResult.has_value());
        EXPECT_EQ(getByLogicalNameResult->id, *createFileResult);
        EXPECT_EQ(getByLogicalNameResult->fullLogicalName, "file.txt");
    }

    TEST_F(ServiceIntegrationTest, Create_ReturnsConflict_WhenLogicalNameAlreadyExists) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto firstCreateFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(firstCreateFileResult.has_value());

        auto secondCreateFileResult = m_fileService->create("file.txt", *addUserResult, 222);
        ASSERT_FALSE(secondCreateFileResult.has_value());
        EXPECT_EQ(secondCreateFileResult.error(), ServiceError::Conflict);
    }

    TEST_F(ServiceIntegrationTest, Create_UsesProvidedMaxVersionCount) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111, 25);
        ASSERT_TRUE(createFileResult.has_value());

        auto getByIdResult = m_fileService->getById(*createFileResult);
        ASSERT_TRUE(getByIdResult.has_value());
        EXPECT_EQ(getByIdResult->maxVersionCount, 25u);
    }

    TEST_F(ServiceIntegrationTest, GetById_ReturnsNotFound_WhenFileDoesNotExist) {
        auto getByIdResult = m_fileService->getById(999999);
        ASSERT_FALSE(getByIdResult.has_value());
        EXPECT_EQ(getByIdResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, GetByLogicalName_ReturnsNotFound_WhenFileDoesNotExist) {
        auto getByLogicalNameResult = m_fileService->getByLogicalName("missing.txt");
        ASSERT_FALSE(getByLogicalNameResult.has_value());
        EXPECT_EQ(getByLogicalNameResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, Rename_ChangesLogicalName) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto renameResult = m_fileService->rename(*createFileResult, "renamed.txt");
        ASSERT_TRUE(renameResult.has_value());

        auto oldLookupResult = m_fileService->getByLogicalName("file.txt");
        ASSERT_FALSE(oldLookupResult.has_value());
        EXPECT_EQ(oldLookupResult.error(), ServiceError::NotFound);

        auto newLookupResult = m_fileService->getByLogicalName("renamed.txt");
        ASSERT_TRUE(newLookupResult.has_value());
        EXPECT_EQ(newLookupResult->id, *createFileResult);

        auto getByIdResult = m_fileService->getById(*createFileResult);
        ASSERT_TRUE(getByIdResult.has_value());
        EXPECT_EQ(getByIdResult->fullLogicalName, "renamed.txt");
    }

    TEST_F(ServiceIntegrationTest, Rename_ReturnsNotFound_WhenFileDoesNotExist) {
        auto renameResult = m_fileService->rename(999999, "renamed.txt");
        ASSERT_FALSE(renameResult.has_value());
        EXPECT_EQ(renameResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, Rename_ReturnsConflict_WhenNewLogicalNameAlreadyExists) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto firstCreateFileResult = m_fileService->create("file1.txt", *addUserResult, 111);
        ASSERT_TRUE(firstCreateFileResult.has_value());

        auto secondCreateFileResult = m_fileService->create("file2.txt", *addUserResult, 222);
        ASSERT_TRUE(secondCreateFileResult.has_value());

        auto renameResult = m_fileService->rename(*secondCreateFileResult, "file1.txt");
        ASSERT_FALSE(renameResult.has_value());
        EXPECT_EQ(renameResult.error(), ServiceError::Conflict);
    }

    TEST_F(ServiceIntegrationTest, Remove_DeletesFile) {
        auto addUserResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addUserResult.has_value());

        auto createFileResult = m_fileService->create("file.txt", *addUserResult, 111);
        ASSERT_TRUE(createFileResult.has_value());

        auto removeResult = m_fileService->remove(*createFileResult);
        ASSERT_TRUE(removeResult.has_value());

        auto getByIdResult = m_fileService->getById(*createFileResult);
        ASSERT_FALSE(getByIdResult.has_value());
        EXPECT_EQ(getByIdResult.error(), ServiceError::NotFound);

        auto getByLogicalNameResult = m_fileService->getByLogicalName("file.txt");
        ASSERT_FALSE(getByLogicalNameResult.has_value());
        EXPECT_EQ(getByLogicalNameResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, Remove_ReturnsNotFound_WhenFileDoesNotExist) {
        auto removeResult = m_fileService->remove(999999);
        ASSERT_FALSE(removeResult.has_value());
        EXPECT_EQ(removeResult.error(), ServiceError::NotFound);
    }

} // namespace tests