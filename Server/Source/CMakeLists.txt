set(PROJECT Server)

file(GLOB_RECURSE source_list "*.cpp" "*.hpp")

add_executable(Server ${source_list})

GroupSourcesByFolder(Server ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(Server PUBLIC
	WIN32_LEAN_AND_MEAN
	VC_EXTRALEAN
	NOGDI
)

target_compile_definitions(Server PRIVATE
	OMP_EXPORTS
)

target_link_libraries(Server PUBLIC
	OMP-SDK
	NetCode
)

target_link_libraries(Server PRIVATE
	CONAN_PKG::nlohmann_json
	CONAN_PKG::cpp-httplib
	CONAN_PKG::openssl
	CONAN_PKG::cxxopts
)

if(MSVC)
	target_link_libraries(Server PRIVATE
		mpr
		shlwapi
		winmm
	)
else()
	target_link_libraries(Server PRIVATE
		dl
		stdc++fs
	)
endif()


target_compile_definitions(Server PRIVATE
	OMP_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
	OMP_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
	OMP_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
)

if(DEFINED ENV{OMP_BUILD_VERSION})
	message("open.mp server build version: $ENV{OMP_BUILD_VERSION}")
	target_compile_definitions(Server PUBLIC BUILD_NUMBER=$ENV{OMP_BUILD_VERSION})
endif()

if(DEFINED ENV{OMP_BUILD_COMMIT})
	message("open.mp server build commit: $ENV{OMP_BUILD_COMMIT}")
	target_compile_definitions(Server PUBLIC OMP_VERSION_HASH=$ENV{OMP_BUILD_COMMIT})
endif()

set_property(TARGET Server PROPERTY OUTPUT_NAME omp-server)
set_property(TARGET Server PROPERTY FOLDER "Server")
set_property(TARGET Server PROPERTY ENABLE_EXPORTS ON)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Server)
set_property(TARGET Server PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
