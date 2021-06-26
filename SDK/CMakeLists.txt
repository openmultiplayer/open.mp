set(PROJECT SDK)
add_library(SDK INTERFACE)

file(GLOB sdk_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

set_property(TARGET SDK PROPERTY SOURCES ${sdk_source_list})

GroupSourcesByFolder(SDK)

set_property(TARGET SDK PROPERTY OUTPUT_NAME omp-sdk)
set_property(TARGET SDK PROPERTY FOLDER "SDK")