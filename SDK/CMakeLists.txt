cmake_minimum_required(VERSION 3.19)
project(OMP-SDK LANGUAGES CXX DESCRIPTION "Stable classes to write open.mp components with" VERSION 1)

# Conan third-party libraries
include(lib/cmake-conan/conan.cmake)
include(lib/cmake-conan/conan-omp.cmake)

conan_omp_add_lib(glm 0.9.9.8)
conan_omp_add_lib(robin-hood-hashing 3.11.5)
conan_omp_add_lib(span-lite 0.10.3)
conan_omp_add_lib(string-view-lite 1.6.0)

add_library(OMP-SDK INTERFACE)

if(MSVC)
	target_compile_options(OMP-SDK INTERFACE
		"/fp:strict"
		"/arch:SSE2"
	)
elseif(UNIX)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
		target_compile_options(OMP-SDK INTERFACE
			-msse2
			-mfpmath=sse
		)
	endif()
endif()

if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	target_compile_options(OMP-SDK INTERFACE
		-Werror=format
	)
endif()

target_link_libraries(OMP-SDK INTERFACE
	CONAN_PKG::glm
	CONAN_PKG::robin-hood-hashing
	CONAN_PKG::span-lite
	CONAN_PKG::string-view-lite
)

target_compile_definitions(OMP-SDK INTERFACE
	GLM_FORCE_QUAT_DATA_WXYZ
	NOMINMAX
	nssv_CONFIG_SELECT_STRING_VIEW=nssv_STRING_VIEW_NONSTD
	span_CONFIG_SELECT_SPAN=span_SPAN_NONSTD
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
	target_compile_definitions(OMP-SDK INTERFACE GLM_FORCE_SSE2)
endif()

target_include_directories(OMP-SDK INTERFACE include/)

file(GLOB_RECURSE omp_sdk_source_list "*.hpp")

set_property(TARGET OMP-SDK PROPERTY SOURCES ${omp_sdk_source_list})
