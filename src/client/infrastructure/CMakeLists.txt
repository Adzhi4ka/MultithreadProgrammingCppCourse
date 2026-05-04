add_subdirectory(api)

add_library(client_infrastructure INTERFACE)

target_link_libraries(client_infrastructure
    INTERFACE
        client_infrastructure_api
)