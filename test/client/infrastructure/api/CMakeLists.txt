add_executable(test_client_infrastructure_api
    ../../common/test-main.cpp
    api-client-test.cpp
    remote-api-test.cpp
)

target_link_libraries(test_client_infrastructure_api
    PRIVATE
        GTest::gtest
        client_test_common
        client_infrastructure_api
        client_domain_models
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        Qt6::Test
)

target_compile_features(test_client_infrastructure_api
    PRIVATE
        cxx_std_23
)

gtest_discover_tests(test_client_infrastructure_api
    DISCOVERY_MODE PRE_TEST
    PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
