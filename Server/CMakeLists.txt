set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
	$<IF:$<CONFIG:Debug>,${CMAKE_BINARY_DIR}/Output/Debug/Server,$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Output/Release/Server,$<IF:$<CONFIG:RelWithDebInfo>,${CMAKE_BINARY_DIR}/Output/RelWithDebInfo/Server,$<IF:$<CONFIG:MinSizeRel>,${CMAKE_BINARY_DIR}/Output/MinSizeRel/Server,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>>>>
)

function(add_server_component name)
	project(${name})

	file(GLOB_RECURSE component_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

	add_library(${PROJECT_NAME} SHARED ${component_source_list})

	target_link_libraries(${PROJECT_NAME} PRIVATE
		SDK
		Server
	)

	GroupSourcesByFolder(${PROJECT_NAME})

	set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${PROJECT_NAME})
	set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Server/Components")
	set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/components)
	set_property(TARGET ${PROJECT_NAME} PROPERTY PREFIX "")
endfunction()

add_subdirectory(Source)
add_subdirectory(Components)