set(PROJECT PacketTracker)

file(GLOB_RECURSE plugin_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(PacketTracker SHARED ${plugin_source_list})

target_include_directories(PacketTracker PRIVATE
  ${CMAKE_SOURCE_DIR}/SDK
)

target_link_libraries(PacketTracker PUBLIC
	SDK
	pawn-runtime
)

GroupSourcesByFolder(PacketTracker)

set_property(TARGET PacketTracker PROPERTY OUTPUT_NAME PacketTracker)
set_property(TARGET PacketTracker PROPERTY FOLDER "Server/Components")
