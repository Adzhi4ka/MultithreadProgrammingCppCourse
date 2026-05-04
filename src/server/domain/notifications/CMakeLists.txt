add_library(server_domain_notifications INTERFACE)

target_include_directories(server_domain_notifications
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_domain_notifications
    INTERFACE
        server_infrastructure_event
)
