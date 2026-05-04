add_subdirectory(models)

add_library(client_domain INTERFACE)

target_link_libraries(client_domain
    INTERFACE
        client_domain_models
)