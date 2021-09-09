# Conan third-party libraries

if (NOT CROSS_BUILD)
	if (MSVC_CXX_ARCHITECTURE_ID)
		string(TOLOWER ${MSVC_CXX_ARCHITECTURE_ID} LOWERCASE_CMAKE_SYSTEM_PROCESSOR)
		if (LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(x64|x86_64|amd64)")
			set(CROSS_BUILD_ARCH x86_64)
		elseif (LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86)")
			set(CROSS_BUILD_ARCH x86)
		else ()
			message(FATAL_ERROR "Unknown CPU '${LOWERCASE_CMAKE_SYSTEM_PROCESSOR}'")
		endif ()
	else ()
		set(CROSS_BUILD_ARCH x86)
	endif ()
endif()

include(cmake-conan/conan.cmake)

# Automatically download and set up paths to a library on Conan
# Works for multi-configuration
function(add_conan_lib pkg_name conan_pkg)
	set(CONAN_DISABLE_CHECK_COMPILER TRUE)

	conan_check(REQUIRED)

	conan_cmake_run(
		REQUIRES ${conan_pkg}
		ARCH ${CROSS_BUILD_ARCH}
		BASIC_SETUP CMAKE_TARGETS
		BUILD missing
	)

	set_target_properties(${pkg_name} PROPERTIES IMPORTED_GLOBAL TRUE)
endfunction()

# Automatically download and set up paths to a library on Conan
# Works for multi-configuration
function(add_conan_lib_opt pkg_name conan_pkg opt)
	set(CONAN_DISABLE_CHECK_COMPILER TRUE)

	conan_check(REQUIRED)

	conan_cmake_run(
		REQUIRES ${conan_pkg}
		ARCH ${CROSS_BUILD_ARCH}
		BASIC_SETUP CMAKE_TARGETS
		BUILD missing
		OPTIONS ${${opt}}
	)

	set_target_properties(${pkg_name} PROPERTIES IMPORTED_GLOBAL TRUE)
endfunction()

# PAWN runtime
# ------------
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(PAWN_RUNTIME_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pawn/source/amx")
	add_library(pawn-runtime STATIC
		${PAWN_RUNTIME_SRC_DIR}/amx.c
		${PAWN_RUNTIME_SRC_DIR}/amxargs.c
		${PAWN_RUNTIME_SRC_DIR}/amxaux.c
		${PAWN_RUNTIME_SRC_DIR}/amxcons.c
		${PAWN_RUNTIME_SRC_DIR}/amxcore.c
		${PAWN_RUNTIME_SRC_DIR}/amxfile.c
		${PAWN_RUNTIME_SRC_DIR}/amxstring.c
		${PAWN_RUNTIME_SRC_DIR}/amxtime.c
		${PAWN_RUNTIME_SRC_DIR}/amxfloat.c
	)
	target_compile_options(pawn-runtime
		PRIVATE "-Wno-unused-function"
	)
	target_include_directories(pawn-runtime 
		PRIVATE ${PAWN_RUNTIME_SRC_DIR}
		INTERFACE "${PAWN_RUNTIME_SRC_DIR}/.."
	)

	include(CheckIncludeFile)
	check_include_file("unistd.h" HAVE_UNISTD_H)
	if(HAVE_UNISTD_H)
		target_compile_definitions(pawn-runtime PUBLIC -DHAVE_UNISTD_H)
	endif()
	check_include_file("inttypes.h" HAVE_INTTYPES_H)
	if(HAVE_INTTYPES_H)
		target_compile_definitions(pawn-runtime PUBLIC -DHAVE_INTTYPES_H)
	endif()
	check_include_file("stdint.h" HAVE_STDINT_H)
	if(HAVE_STDINT_H)
		target_compile_definitions(pawn-runtime PUBLIC -DHAVE_STDINT_H)
	endif()
	check_include_file("alloca.h" HAVE_ALLOCA_H)
	if(HAVE_ALLOCA_H)
		target_compile_definitions(pawn-runtime PUBLIC -DHAVE_ALLOCA_H)
	endif()

	if (MSVC)
		target_compile_definitions(pawn-runtime
			PRIVATE
				-D_CRT_SECURE_NO_WARNINGS
		)
	endif()

	target_compile_definitions(pawn-runtime
		PRIVATE
			-DFLOATPOINT
			-DAMX_STRING_LIB
	)

	if(UNIX)
		# TODO: set fvisibility for gcc/clang
		target_compile_definitions(pawn-runtime PUBLIC -DLINUX)
		check_include_file("ffi.h" HAVE_FFI_H)
		target_include_directories(pawn-runtime PUBLIC "${PAWN_RUNTIME_SRC_DIR}/../linux")
		target_sources(pawn-runtime PRIVATE "${PAWN_RUNTIME_SRC_DIR}/../linux/getch.c")
	endif()

	if(WIN32)
		target_compile_definitions(pawn-runtime
			PRIVATE
				-D_CRT_SECURE_NO_WARNINGS
				-D_CRT_NONSTDC_NO_DEPRECATE
		)
		if(NOT BORLAND)
			target_link_libraries(pawn-runtime PRIVATE winmm)
		endif()
	endif()

	set_property(TARGET pawn-runtime PROPERTY FOLDER "lib")
	set_property(TARGET pawn-runtime PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# RakNet
# ------
add_subdirectory(RakNet)
set_property(TARGET raknet PROPERTY FOLDER "lib")
target_compile_options(raknet
	PRIVATE
		"-Wno-logical-op-parentheses"
		"-Wno-bitwise-op-parentheses"
		"-Wno-#pragma-messages"
)

# ttmath
# ------
add_subdirectory(ttmath)
GroupSourcesByFolder(ttmath)
set_property(TARGET ttmath PROPERTY FOLDER "lib")

# GLM
# ---
add_conan_lib(CONAN_PKG::glm glm/0.9.9.8)

# JSON
# ----
add_conan_lib(CONAN_PKG::nlohmann_json nlohmann_json/3.9.1)

# Abseil
# ------
add_conan_lib(CONAN_PKG::abseil abseil/20210324.2)

# SQLite 3
# --------
add_conan_lib(CONAN_PKG::sqlite3 sqlite3/3.36.0)

# cpp-httplib
# ---
add_conan_lib(CONAN_PKG::cpp-httplib cpp-httplib/0.9.0)

# OpenSSL
# -------
add_conan_lib_opt(CONAN_PKG::openssl openssl/1.1.1k openssl:shared=False)

# cxxopts
# ----
add_conan_lib(CONAN_PKG::cxxopts cxxopts/2.2.1)
