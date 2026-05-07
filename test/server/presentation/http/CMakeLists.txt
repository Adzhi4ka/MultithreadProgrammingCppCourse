if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS system json)

add_executable(test_server_presentation_http
    controller-test-integration.cpp
    auth-controller-test.cpp
    group-controller-test.cpp
    file-controller-test.cpp
    file-version-controller-test.cpp
    file-lock-controller-test.cpp
)

target_link_libraries(test_server_presentation_http
    PRIVATE
        GTest::gtest_main
        server_presentation
        server_domain
        server_infrastructure
        Boost::json
)

gtest_discover_tests(test_server_presentation_http)