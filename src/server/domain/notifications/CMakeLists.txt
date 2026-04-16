add_library(server_domain_notification INTERFACE)

target_include_directories(server_domain_notification
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_domain_notification
    INTERFACE
        server_domain_models
        server_infrastructure
)