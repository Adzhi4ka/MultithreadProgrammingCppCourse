add_executable(test_server_infrastructure_database_sqlite
    sqlite-database-test.cpp
)

target_link_libraries(test_server_infrastructure_database_sqlite
    PRIVATE
        GTest::gtest_main
        server_infrastructure_database_sqlite
)

gtest_discover_tests(test_server_infrastructure_database_sqlite)