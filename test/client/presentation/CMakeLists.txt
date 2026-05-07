add_executable(test_client_presentation
    ../common/test-main.cpp
    file-tree-widget-test.cpp
    login-dialog-test.cpp
    main-window-test.cpp
    editor-window-test.cpp
)

target_link_libraries(test_client_presentation
    PRIVATE
        GTest::gtest
        client_test_common
        client_domain
        client_infrastructure
        client_application
        client_presentation
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        Qt6::Test
)

target_compile_features(test_client_presentation
    PRIVATE
        cxx_std_23
)

gtest_discover_tests(test_client_presentation
    DISCOVERY_MODE PRE_TEST
    PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
