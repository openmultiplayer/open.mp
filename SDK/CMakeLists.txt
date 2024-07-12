cmake_minimum_required(VERSION 3.19)
project(OMP-SDK LANGUAGES CXX DESCRIPTION "Stable classes to write open.mp components with" VERSION 1)

add_subdirectory(lib/glm)
add_subdirectory(lib/robin-hood-hashing)
add_subdirectory(lib/span-lite)
add_subdirectory(lib/string-view-lite)

add_library(OMP-SDK INTERFACE)

if(MSVC)
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		target_compile_options(OMP-SDK INTERFACE
			"/fp:strict"
			"/arch:SSE2"
		)
	else()
		target_compile_options(OMP-SDK INTERFACE
			"/fp:strict"
		)
	endif()
elseif(UNIX)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
		target_compile_options(OMP-SDK INTERFACE
			-msse2
			-mfpmath=sse
		)
	else()
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
			target_compile_options(OMP-SDK INTERFACE "-mfpu=neon")
		endif()
		if(TARGET_BUILD_ARCH)
			target_compile_options(OMP-SDK INTERFACE "-march=${TARGET_BUILD_ARCH}")
		endif()
	endif()
endif()

if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	target_compile_options(OMP-SDK INTERFACE
		-Werror=format
	)
endif()

target_link_libraries(OMP-SDK INTERFACE
	glm::glm
	robin_hood
	span-lite
	string-view-lite
)

target_compile_definitions(OMP-SDK INTERFACE
	GLM_FORCE_QUAT_DATA_WXYZ
	NOMINMAX
	nssv_CONFIG_SELECT_STRING_VIEW=nssv_STRING_VIEW_NONSTD
	span_CONFIG_SELECT_SPAN=span_SPAN_NONSTD
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
	target_compile_definitions(OMP-SDK INTERFACE GLM_FORCE_SSE2)
else()
	target_compile_definitions(OMP-SDK INTERFACE GLM_FORCE_NEON)
endif()

target_include_directories(OMP-SDK INTERFACE include/)

file(GLOB_RECURSE omp_sdk_source_list "*.hpp")

set_property(TARGET OMP-SDK PROPERTY SOURCES ${omp_sdk_source_list})
