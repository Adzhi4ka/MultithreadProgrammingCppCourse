add_library(client_test_common STATIC
    mock-http-server.cpp
    qt-test-helpers.cpp
)

target_include_directories(client_test_common
    PUBLIC
        ${PROJECT_SOURCE_DIR}/test/client
        ${PROJECT_SOURCE_DIR}/src/client
)

target_link_libraries(client_test_common
    PUBLIC
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        Qt6::Test
        GTest::gtest
)

target_compile_features(client_test_common
    PUBLIC
        cxx_std_23
)
