add_subdirectory(models)
add_subdirectory(services)

add_library(client_domain INTERFACE)

target_link_libraries(client_domain
    INTERFACE
        client_domain_models
        client_domain_services
)
