set(PROJECT PawnPlugin)
add_library(PawnPlugin SHARED)

file(GLOB_RECURSE plugin_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

target_include_directories(PawnPlugin PRIVATE
  ${CMAKE_SOURCE_DIR}/SDK
)

target_link_libraries(PawnPlugin PUBLIC
	SDK
	pawn-runtime
)

set_property(TARGET PawnPlugin PROPERTY SOURCES ${plugin_source_list})

GroupSourcesByFolder(PawnPlugin)

set_property(TARGET PawnPlugin PROPERTY OUTPUT_NAME PawnPlugin)
set_property(TARGET PawnPlugin PROPERTY FOLDER "Server/Components")
