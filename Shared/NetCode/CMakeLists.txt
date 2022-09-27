project(OMP-NetCode)

add_library(OMP-NetCode INTERFACE)

target_link_libraries(OMP-NetCode INTERFACE OMP-Network)

target_include_directories(OMP-NetCode INTERFACE .)

file(GLOB_RECURSE netcode_source_list "*.hpp")

set_property(TARGET OMP-NetCode PROPERTY SOURCES ${netcode_source_list})
set_property(TARGET OMP-NetCode PROPERTY POSITION_INDEPENDENT_CODE ON)

GroupSourcesByFolder(OMP-NetCode)
