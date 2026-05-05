find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)

qt_standard_project_setup()

add_subdirectory(domain)
add_subdirectory(infrastructure)
add_subdirectory(presentation)

add_library(file_storage_client_core INTERFACE)

target_link_libraries(file_storage_client_core
    INTERFACE
        client_domain
        client_infrastructure
        client_presentation
)

add_executable(file_storage_qt_client
    main.cpp
)

target_link_libraries(file_storage_qt_client
    PRIVATE
        file_storage_client_core
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
)

target_compile_features(file_storage_qt_client
    PRIVATE
        cxx_std_23
)
