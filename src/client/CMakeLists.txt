find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)

qt_standard_project_setup()

add_subdirectory(domain)
add_subdirectory(infrastructure)
add_subdirectory(application)
add_subdirectory(presentation)

add_executable(client_app
    main.cpp
)

target_link_libraries(client_app
    PRIVATE
        client_domain
        client_infrastructure
        client_application
        client_presentation
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
)
