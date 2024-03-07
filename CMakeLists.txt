cmake_minimum_required(VERSION 3.19)
project(open-mp LANGUAGES C CXX VERSION 1.2.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT ${CMAKE_CXX_COMPILER_ID} MATCHES "(Clang)")
	message(
		FATAL_ERROR
		"Clang is the only compiler officially supported for building."
		"Use clang on Unix or clang-cl on Windows to build."
	)
endif()

set(CMAKE_CXX_STANDARD 17)

# MSVC
if(MSVC)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	add_link_options("/LARGEADDRESSAWARE")
	# Use strict intermediate floating-point precision
	add_compile_options("/fp:strict")
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		add_compile_options("/arch:SSE2")
	endif()
	add_compile_options("-Werror=format")

elseif(UNIX)
	# Linux GCC/Clang
	if(NOT APPLE)
		set(CMAKE_CXX_STDLIB "libstdc++" CACHE STRING "The standard library to use.")
		set(TARGET_BUILD_ARCH "x86" CACHE STRING "The architecture to build for.")
		set(SHARED_OPENSSL TRUE CACHE BOOL "Whether to dynamically link the OpenSSL library")
		set(STATIC_STDCXX FALSE CACHE BOOL "Whether to statically link against libc++.")
	# Apple Clang
	else()
		set(CMAKE_CXX_STDLIB "libc++" CACHE STRING "The standard library to use.")

		if (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64)")
			set(TARGET_BUILD_ARCH armv8)
		else ()
			set(TARGET_BUILD_ARCH x86_64)
		endif ()

		set(CMAKE_THREAD_LIBS_INIT "-lpthread")
		set(CMAKE_HAVE_THREADS_LIBRARY 1)
		set(CMAKE_USE_WIN32_THREADS_INIT 0)
		set(CMAKE_USE_PTHREADS_INIT 1)
		set(THREADS_PREFER_PTHREAD_FLAG ON)
	endif()

	set(CMAKE_C_FLAGS "-pthread -Werror=format -fno-builtin-getentropy")
	set(CMAKE_CXX_FLAGS "-fno-sized-deallocation -std=gnu++17 -Wno-register -pthread -Werror=format -fno-builtin-getentropy")

	if(STATIC_STDCXX)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libstdc++")
	endif()

	# If not cross-building, assume we're using a x86_64 compiler
	if(TARGET_BUILD_ARCH)
		message("Generating ${TARGET_BUILD_ARCH} project")
	else ()
		message(WARNING "TARGET_BUILD_ARCH not provided; generating a project based on host's arch")
	endif ()

	# Clang or Apple Clang
	if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=${CMAKE_CXX_STDLIB} -fvisibility=hidden -Wno-switch")
	endif()

	if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
		# Use strict intermediate floating-point precision
		add_compile_options(-msse2 -mfpmath=sse)
	endif()

	if(TARGET_BUILD_ARCH MATCHES "(i[3-6]86|x86)" AND NOT TARGET_BUILD_ARCH MATCHES "(x64|x86_64|amd64)")
		set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
		set(CMAKE_SIZEOF_VOID_P 4)
	endif ()

endif()

# Flags to know when we are compiling in Debug mode.
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -D_DO_PRINTF")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -D_DO_PRINTF")

if(WIN32)
	# https://stackoverflow.com/questions/42946335/deprecated-header-codecvt-replacement
	# `codecvt` is deprecated, but is defined as remaining until a more secure replacement is
	# standardised, thus just defining `_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING` is fine.
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
		-DWIN32_LEAN_AND_MEAN
		-D_CRT_DECLARE_NONSTDC_NAMES
		-D_CRT_NONSTDC_NO_WARNINGS
	)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(GroupSourcesByFolder target)
	set(SOURCE_GROUP_DELIMITER "/")
	set(last_dir "")
	set(files "")

	get_target_property(sources ${target} SOURCES)

	foreach(file ${sources})
		file(RELATIVE_PATH relative_file "${CMAKE_CURRENT_SOURCE_DIR}" ${file})
		get_filename_component(dir "${relative_file}" PATH)
		if(NOT "${dir}" STREQUAL "${last_dir}")
			if(files)
				source_group("${last_dir}" FILES ${files})
			endif()
			set(files "")
		endif()
		set(files ${files} ${file})
		set(last_dir "${dir}")
	endforeach()

	if(files)
		source_group("${last_dir}" FILES ${files})
	endif()
endfunction()

set(BUILD_SERVER TRUE CACHE BOOL "Whether to build the open.mp server")
set(BUILD_PAWN_COMPONENT TRUE CACHE BOOL "Whether to build the PAWN component")
set(BUILD_UNICODE_COMPONENT TRUE CACHE BOOL "Whether to build the Unicode component")
set(BUILD_LEGACY_COMPONENTS TRUE CACHE BOOL "Whether to build the legacy components")
set(BUILD_TEST_COMPONENTS FALSE CACHE BOOL "Whether to build the test component")
set(BUILD_SQLITE_COMPONENT TRUE CACHE BOOL "Whether to build the SQLite component")
set(BUILD_FIXES_COMPONENT TRUE CACHE BOOL "Whether to build the Fixes component")

if (UNIX)
	set(BUILD_ABI_CHECK_TOOL TRUE CACHE BOOL "Whether to build the abi-check tool")
endif()

add_subdirectory(lib)

if(BUILD_SERVER)
	message("Configuring server")
	add_subdirectory(SDK)
	add_subdirectory(Shared)
	add_subdirectory(Server)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Server)
endif()

if(BUILD_ABI_CHECK_TOOL)
	message("Configuring abi-check")
	add_subdirectory(Tools)
endif()
