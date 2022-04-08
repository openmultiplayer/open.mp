set(PROJECT Shared)

add_library(NetCode STATIC)

target_link_libraries(NetCode PUBLIC OMP-SDK)

target_include_directories(NetCode PUBLIC .)

file(GLOB_RECURSE netcode_source_list "*.cpp" "*.hpp")

set_property(TARGET NetCode PROPERTY SOURCES ${netcode_source_list})
set_property(TARGET NetCode PROPERTY POSITION_INDEPENDENT_CODE ON)

GroupSourcesByFolder(NetCode)
