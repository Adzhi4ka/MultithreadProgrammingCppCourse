add_executable(test_server_domain_services
    service-test-integration.cpp
    user-service-test.cpp
    group-service-test.cpp
    file-acl-service-test.cpp
    file-service-test.cpp
    file-version-service-test.cpp
)

target_link_libraries(test_server_domain_services
    PRIVATE
        GTest::gtest_main
        server_domain
        server_infrastructure_security
)

gtest_discover_tests(test_server_domain_services)