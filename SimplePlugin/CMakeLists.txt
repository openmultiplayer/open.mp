set(PROJECT SimplePlugin)
add_library(SimplePlugin SHARED)

file(GLOB_RECURSE simple_plugin_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

target_include_directories(SimplePlugin PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../SDK"
)

set_property(TARGET SimplePlugin PROPERTY SOURCES ${simple_plugin_source_list})

GroupSourcesByFolder(SimplePlugin)

set_property(TARGET SimplePlugin PROPERTY OUTPUT_NAME SimplePlugin)
set_property(TARGET SimplePlugin PROPERTY FOLDER "Server/Components")