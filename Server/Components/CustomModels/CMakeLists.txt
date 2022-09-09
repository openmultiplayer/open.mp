get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})
include_directories(${CMAKE_SOURCE_DIR}/lib/cpp-httplib)

target_link_libraries(${ProjectId} PRIVATE
    CONAN_PKG::ghc-filesystem
)
