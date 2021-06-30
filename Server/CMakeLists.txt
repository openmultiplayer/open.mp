set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
	$<IF:$<CONFIG:Debug>,${CMAKE_BINARY_DIR}/Output/Debug/Server,$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Output/Release/Server,$<IF:$<CONFIG:RelWithDebInfo>,${CMAKE_BINARY_DIR}/Output/RelWithDebInfo/Server,$<IF:$<CONFIG:MinSizeRel>,${CMAKE_BINARY_DIR}/Output/MinSizeRel/Server,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>>>>
)

add_subdirectory(Source)
add_subdirectory(Components)