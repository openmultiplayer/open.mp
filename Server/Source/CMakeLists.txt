set(PROJECT Server)

file(GLOB_RECURSE source_list "*.cpp" "*.hpp")

add_executable(Server ${source_list})

GroupSourcesByFolder(Server ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(Server PUBLIC
	WIN32_LEAN_AND_MEAN
	VC_EXTRALEAN
	NOGDI
	OMP_EXPORTS
)

target_link_libraries(Server PUBLIC
	SDK
	raknet
)

if(MSVC)
	target_link_libraries(Server PRIVATE
		mpr
		shlwapi
	)
else()
	target_link_libraries(Server PRIVATE
		dl
	)
endif()

set_property(TARGET Server PROPERTY OUTPUT_NAME omp-server)
set_property(TARGET Server PROPERTY FOLDER "Server")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Server)
