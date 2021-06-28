cmake_minimum_required(VERSION 3.19)
project(open-mp LANGUAGES C CXX VERSION 0.0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_BUILD_TYPE Debug)


if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	message(
		FATAL_ERROR
		"Clang is the only compiler officially supported for building."
		"Use clang on Unix or clang-cl on Windows to build."
	)
endif()

# MSVC
if(MSVC)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	add_compile_options("/clang:-v")
	add_link_options("/LARGEADDRESSAWARE")
	# Use strict intermediate floating-point precision
	add_compile_options("/fp:strict")

elseif(UNIX)
	# Linux GCC/Clang
	if(NOT APPLE)
		set(CMAKE_CXX_STDLIB "libstdc++" CACHE STRING "The standard library to use.")
		set(CROSS_BUILD FALSE CACHE BOOL "Whether to cross-build.")
		set(CROSS_BUILD_ARCH "armv6" CACHE STRING "The architecture to cross-build for.")
	# Apple Clang
	else()
		set(CMAKE_CXX_STDLIB "libc++" CACHE STRING "The standard library to use.")

		set(CROSS_BUILD FALSE)
		set(CROSS_BUILD_ARCH)

		set(CMAKE_THREAD_LIBS_INIT "-lpthread")
		set(CMAKE_HAVE_THREADS_LIBRARY 1)
		set(CMAKE_USE_WIN32_THREADS_INIT 0)
		set(CMAKE_USE_PTHREADS_INIT 1)
		set(THREADS_PREFER_PTHREAD_FLAG ON)
	endif()

	set(CMAKE_C_FLAGS "-pthread")
	set(CMAKE_CXX_FLAGS "-fno-sized-deallocation -std=gnu++17 -Wno-register -pthread -static-libstdc++")
	
	# If not cross-building, assume we're using a x86_64 compiler
	if(CROSS_BUILD)
		set(CMAKE_CXX_FLAGS "-march=${CROSS_BUILD_ARCH} ${CMAKE_CXX_FLAGS}")
		message("Generating cross-build project for ${CROSS_BUILD_ARCH}.")
	else()
		set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
		message("Generating x86 project")
	endif()
	# Clang or Apple Clang
	if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=${CMAKE_CXX_STDLIB} -fvisibility=hidden -Wno-switch -Wno-unused-command-line-argument")
	endif()

	# Use strict intermediate floating-point precision
	add_compile_options(-msse2 -mfpmath=sse)

endif()

# set up configuration types
if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;DebugPreprocess" CACHE STRING "" FORCE)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build.")
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;DebugPreprocess")
endif()
if(MSVC)
	set(flags_DebugPreprocess "/P")
else()
	set(flags_DebugPreprocess "-E")
endif()

function(enable_warnings proj)
	# GLM_FORCE_SILENT_WARNINGS Disables warnings in GLM.
	# 4100 is `unreferenced formal parameter` in MSVC.
	if(MSVC)
		target_compile_options(${proj} PUBLIC /W4 /WX /wd4100 /wd4307 -DGLM_FORCE_SILENT_WARNINGS)
	else()
		target_compile_options(${proj} PUBLIC -fpermissive -DGLM_FORCE_SILENT_WARNINGS)
		if (CROSS_BUILD)
			target_compile_options(${proj} PUBLIC -Wno-attributes)
		endif()
	endif()
endfunction()

# set up special DebugPreprocess type (will preprocess files and output the result)
set(CMAKE_C_FLAGS_DEBUGPREPROCESS "${CMAKE_C_FLAGS_DEBUG} ${flags_DebugPreprocess}")
set(CMAKE_CXX_FLAGS_DEBUGPREPROCESS "${CMAKE_CXX_FLAGS_DEBUG} ${flags_DebugPreprocess}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUGPREPROCESS ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUGPREPROCESS ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_MODULE_LINKER_FLAGS_DEBUGPREPROCESS ${CMAKE_MODULE_LINKER_FLAGS_DEBUG})

# Flags to know when we are compiling in Debug mode.
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DO_PRINTF")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DO_PRINTF")

if(WIN32)
	# https://stackoverflow.com/questions/42946335/deprecated-header-codecvt-replacement
	# `codecvt` is deprecated, but is defined as remaining until a more secure replacement is
	# standardised, thus just defining `_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING` is fine.
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
		-DWIN32_LEAN_AND_MEAN
	)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

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

add_subdirectory(lib)
add_subdirectory(SDK)

set(PROJECT Server)
add_executable(Server "")

# enable_warnings(Server)

file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/Server/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Server/*.hpp")

set (EXCLUDE_COMPONENTS_DIR "Server/Components")
foreach (TMP_PATH ${source_list})
	string (FIND ${TMP_PATH} ${EXCLUDE_COMPONENTS_DIR} EXCLUDE_DIR_FOUND)
	if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
		list (REMOVE_ITEM source_list ${TMP_PATH})
	endif ()
endforeach(TMP_PATH)

set_property(TARGET Server PROPERTY SOURCES ${source_list})

GroupSourcesByFolder(Server ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(lib)
include_directories(SDK)

############# Components (plugins) #############
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	add_subdirectory(Server/Components/PawnPlugin)
endif()

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
