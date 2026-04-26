find_package(Boost REQUIRED COMPONENTS system json)

add_library(server_presentation_http STATIC
    auth-controller.cpp
    notification-controller.cpp
    group-controller.cpp
    file-acl-controller.cpp
    file-lock-controller.cpp
    file-controller.cpp
    file-version-controller.cpp
)

target_include_directories(server_presentation_http
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_presentation_http
    PUBLIC
        server_infrastructure
        server_domain
        Boost::json
)