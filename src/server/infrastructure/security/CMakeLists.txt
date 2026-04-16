add_library(server_infrastructure_security INTERFACE)

find_package(PkgConfig REQUIRED)
pkg_search_module(LIBSODIUM REQUIRED libsodium)

target_link_libraries(server_infrastructure_security
    INTERFACE
        ${LIBSODIUM_LINK_LIBRARIES}
)

target_include_directories(server_infrastructure_security
    INTERFACE
        ${LIBSODIUM_INCLUDE_DIRS}
)