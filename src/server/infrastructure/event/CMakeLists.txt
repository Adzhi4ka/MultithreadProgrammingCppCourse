find_package(Boost REQUIRED)

add_library(server_infrastructure_event INTERFACE)

target_include_directories(server_infrastructure_event
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_event INTERFACE
    Boost::boost
)