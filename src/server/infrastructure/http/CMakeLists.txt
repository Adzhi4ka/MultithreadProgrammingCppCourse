if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS system)

add_library(server_infrastructure_http STATIC
    router.cpp
    route-context.cpp
    http-session.cpp
    listener.cpp
    sse-session.cpp
    active-session-registry.cpp
)

target_include_directories(server_infrastructure_http
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/server
)

target_link_libraries(server_infrastructure_http
    PUBLIC
        Boost::boost
        Boost::system
)