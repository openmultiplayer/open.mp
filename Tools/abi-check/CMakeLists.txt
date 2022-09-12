set(PROJECT abi-check)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
	$<IF:$<CONFIG:Debug>,${CMAKE_BINARY_DIR}/Output/Debug/Tools,$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Output/Release/Tools,$<IF:$<CONFIG:RelWithDebInfo>,${CMAKE_BINARY_DIR}/Output/RelWithDebInfo/Tools,$<IF:$<CONFIG:MinSizeRel>,${CMAKE_BINARY_DIR}/Output/MinSizeRel/Tools,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>>>>
)

file(GLOB_RECURSE source_list "*.cpp" "*.hpp")

add_executable(abi-check ${source_list})

GroupSourcesByFolder(abi-check ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(abi-check PUBLIC
	WIN32_LEAN_AND_MEAN
	VC_EXTRALEAN
	NOGDI
)

target_link_libraries(abi-check PRIVATE
	CONAN_PKG::libelfin
	CONAN_PKG::cxxopts
)

if(MSVC)
	target_link_libraries(abi-check PRIVATE
		mpr
		shlwapi
		winmm
	)
else()
	target_link_libraries(abi-check PRIVATE dl)
endif()

set_property(TARGET abi-check PROPERTY OUTPUT_NAME abi-check)
set_property(TARGET abi-check PROPERTY FOLDER "abi-check")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT abi-check)
set_property(TARGET abi-check PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
