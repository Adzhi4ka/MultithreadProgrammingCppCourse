add_subdirectory(models)
add_subdirectory(services)

add_library(server_domain INTERFACE)

target_link_libraries(server_domain
    INTERFACE
        server_domain_models
        server_domain_services
)