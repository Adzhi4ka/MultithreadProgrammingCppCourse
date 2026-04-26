#include "service-test-integration.h"

namespace tests {

    TEST_F(ServiceIntegrationTest, EmptyTest) {
        SUCCEED();
    }

    TEST_F(ServiceIntegrationTest, AddUser_Only) {
        auto addResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addResult.has_value());
        EXPECT_GT(*addResult, 0);
    }

    TEST_F(ServiceIntegrationTest, AddUser_ReturnsUserId_WhenLoginIsUnique) {
        auto addResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(addResult.has_value());

        const int64_t userId = *addResult;
        EXPECT_GT(userId, 0);

        auto loginResult = m_userService->login("ivan", "123456");
        ASSERT_TRUE(loginResult.has_value());
        EXPECT_EQ(*loginResult, userId);
    }

    TEST_F(ServiceIntegrationTest, AddUser_ReturnsConflict_WhenLoginAlreadyExists) {
        auto firstAddResult = m_userService->addUser("ivan", "123456");
        ASSERT_TRUE(firstAddResult.has_value());

        auto secondAddResult = m_userService->addUser("ivan", "abcdef");
        ASSERT_FALSE(secondAddResult.has_value());
        EXPECT_EQ(secondAddResult.error(), ServiceError::Conflict);
    }

    TEST_F(ServiceIntegrationTest, Login_ReturnsUserId_WhenCredentialsAreValid) {
        auto addResult = m_userService->addUser("petya", "qwerty");
        ASSERT_TRUE(addResult.has_value());

        auto loginResult = m_userService->login("petya", "qwerty");
        ASSERT_TRUE(loginResult.has_value());
        EXPECT_EQ(*loginResult, *addResult);
    }

    TEST_F(ServiceIntegrationTest, Login_ReturnsNotFound_WhenUserDoesNotExist) {
        auto loginResult = m_userService->login("missing_user", "123456");
        ASSERT_FALSE(loginResult.has_value());
        EXPECT_EQ(loginResult.error(), ServiceError::NotFound);
    }

    TEST_F(ServiceIntegrationTest, Login_ReturnsForbidden_WhenPasswordIsWrong) {
        auto addResult = m_userService->addUser("vasya", "correct_password");
        ASSERT_TRUE(addResult.has_value());

        auto loginResult = m_userService->login("vasya", "wrong_password");
        ASSERT_FALSE(loginResult.has_value());
        EXPECT_EQ(loginResult.error(), ServiceError::Forbidden);
    }

} // namespace tests