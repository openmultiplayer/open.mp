set(PROJECT Shared)

add_library(NetCode INTERFACE)

target_link_libraries(NetCode INTERFACE SDK)

target_include_directories(NetCode INTERFACE .)

file(GLOB_RECURSE netcode_source_list "*.hpp")

set_property(TARGET NetCode PROPERTY SOURCES ${netcode_source_list})
