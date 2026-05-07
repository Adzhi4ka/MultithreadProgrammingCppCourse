#include "controller-test-integration.h"

namespace tests {

TEST_F(ControllerIntegrationTest, AuthController_RegisterAndLogin_Work) {
    auto registerResult = registerUser("alice", "alice123");
    EXPECT_GT(registerResult.userId, 0);
    EXPECT_EQ(registerResult.login, "alice");
    EXPECT_FALSE(registerResult.token.empty());

    auto loginResult = loginUser("alice", "alice123");
    EXPECT_EQ(loginResult.userId, registerResult.userId);
    EXPECT_EQ(loginResult.login, "alice");
    EXPECT_FALSE(loginResult.token.empty());
}

TEST_F(ControllerIntegrationTest, AuthController_ReturnsUnauthorized_ForProtectedEndpointWithoutToken) {
    auto response = request(http::verb::get, "/api/groups");
    EXPECT_EQ(response.result(), http::status::unauthorized);
}

}  // namespace tests