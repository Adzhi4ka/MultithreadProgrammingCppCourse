find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

add_subdirectory(domain)
add_subdirectory(infrastructure)

add_library(file_storage_client_core INTERFACE)

target_link_libraries(file_storage_client_core
    INTERFACE
        client_domain
        client_infrastructure
)