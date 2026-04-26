#include "controller-test-integration.h"

namespace tests {

    TEST_F(ControllerIntegrationTest, FileLockController_AcquireRenewRelease_Work) {
        auto alice = registerUser("alice", "alice123");

        const auto fileId = createFile(alice.token, "docs/locked.txt");

        json::object acquireBody{
            {"fileId", fileId},
            {"lockDurationSec", 120}
        };

        auto acquireResponse = request(http::verb::post,
                                       "/api/file-locks",
                                       json::serialize(acquireBody),
                                       alice.token);
        ASSERT_EQ(acquireResponse.result(), http::status::created) << acquireResponse.body();

        auto acquiredJson = parseJson(acquireResponse).as_object();
        const auto lockToken = acquiredJson.at("lockToken").as_int64();

        auto activeResponse = request(http::verb::get,
                                      "/api/file-locks/active?fileId=" + std::to_string(fileId),
                                      {},
                                      alice.token);
        ASSERT_EQ(activeResponse.result(), http::status::ok) << activeResponse.body();

        auto activeJson = parseJson(activeResponse).as_object();
        EXPECT_EQ(activeJson.at("fileId").as_int64(), fileId);
        EXPECT_EQ(activeJson.at("lockToken").as_int64(), lockToken);

        json::object renewBody{
            {"fileId", fileId},
            {"lockToken", lockToken},
            {"lockDurationSec", 180}
        };

        auto renewResponse = request(http::verb::put,
                                     "/api/file-locks/renew",
                                     json::serialize(renewBody),
                                     alice.token);
        EXPECT_EQ(renewResponse.result(), http::status::ok);

        auto releaseResponse = request(http::verb::delete_,
                                       "/api/file-locks?fileId=" + std::to_string(fileId) +
                                       "&lockToken=" + std::to_string(lockToken),
                                       {},
                                       alice.token);
        EXPECT_EQ(releaseResponse.result(), http::status::ok);
    }

} // namespace tests