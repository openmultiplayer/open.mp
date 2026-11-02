# Third-party dependencies are managed via vcpkg (see vcpkg.json)

if(BUILD_SERVER)
	if(BUILD_PAWN_COMPONENT)
		# PAWN runtime
		# ------------
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
				-DOVERWRITE_AMX_REGISTER
				-DFLOATPOINT
				-DAMX_STRING_LIB
				-DsNAMEMAX=63
				-DPAWN_CELL_SIZE=32
				-DAMX_FILENO_CHECKS
				-DAMX_NODYNALOAD

				# Disable default amx_FindPublic implementation because we want our own more performant one
				# Also disable amx_Allot because we want to check if we have enough memory when we are passing data
				# Which is more than available memory, so instead of a crash, we let user know about it.
				AMX_ALIGN AMX_CLEANUP AMX_CLONE AMX_DEFCALLBACK AMX_FLAGS AMX_GETADDR
				AMX_INIT AMX_MEMINFO AMX_NAMELENGTH AMX_NATIVEINFO AMX_PUSHXXX
				AMX_RAISEERROR AMX_REGISTER AMX_SETCALLBACK AMX_SETDEBUGHOOK
				AMX_XXXNATIVES AMX_XXXPUBVARS AMX_XXXSTRING AMX_XXXTAGS AMX_XXXUSERDATA AMX_UTF8XXX
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
			target_link_libraries(pawn-runtime PRIVATE winmm)
		endif()

		set_property(TARGET pawn-runtime PROPERTY FOLDER "lib")
		set_property(TARGET pawn-runtime PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif()

if(BUILD_LEGACY_COMPONENTS)
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
endif()

# ttmath
# ------
add_subdirectory(ttmath)
GroupSourcesByFolder(ttmath)
set_property(TARGET ttmath PROPERTY FOLDER "lib")

# JSON
# ----
find_package(nlohmann_json CONFIG REQUIRED)
if(NOT TARGET CONAN_PKG::nlohmann_json)
	add_library(CONAN_PKG::nlohmann_json INTERFACE IMPORTED GLOBAL)
	target_link_libraries(CONAN_PKG::nlohmann_json INTERFACE nlohmann_json::nlohmann_json)
endif()

# OpenSSL
# -------
find_package(OpenSSL REQUIRED)
if(NOT TARGET CONAN_PKG::openssl)
	add_library(CONAN_PKG::openssl INTERFACE IMPORTED GLOBAL)
	target_link_libraries(CONAN_PKG::openssl INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()

# ghc-filesystem
# ----
find_package(ghc_filesystem CONFIG REQUIRED)
if(NOT TARGET CONAN_PKG::ghc-filesystem)
	add_library(CONAN_PKG::ghc-filesystem INTERFACE IMPORTED GLOBAL)
	target_link_libraries(CONAN_PKG::ghc-filesystem INTERFACE ghcFilesystem::ghc_filesystem)
endif()

if(BUILD_SQLITE_COMPONENT)
	# SQLite 3
	# --------
	find_package(SQLite3 REQUIRED)
	if(NOT TARGET CONAN_PKG::sqlite3)
		add_library(CONAN_PKG::sqlite3 INTERFACE IMPORTED GLOBAL)
		if(TARGET SQLite3::SQLite3)
			target_link_libraries(CONAN_PKG::sqlite3 INTERFACE SQLite3::SQLite3)
		elseif(TARGET unofficial::sqlite3::sqlite3)
			target_link_libraries(CONAN_PKG::sqlite3 INTERFACE unofficial::sqlite3::sqlite3)
		endif()
	endif()
endif()

if(BUILD_UNICODE_COMPONENT)
	# ICU
	# ---
	find_package(ICU REQUIRED COMPONENTS uc i18n data)
	if(NOT TARGET CONAN_PKG::icu)
		add_library(CONAN_PKG::icu INTERFACE IMPORTED GLOBAL)
		target_link_libraries(CONAN_PKG::icu INTERFACE ICU::uc ICU::i18n ICU::data)
	endif()

	if(WIN32)
		target_link_options(CONAN_PKG::icu INTERFACE "/SAFESEH:NO")
	endif()
endif()
endif()

if(BUILD_ABI_CHECK_TOOL)
	find_package(libelfin CONFIG REQUIRED)
	if(NOT TARGET CONAN_PKG::libelfin)
		add_library(CONAN_PKG::libelfin INTERFACE IMPORTED GLOBAL)
		target_link_libraries(CONAN_PKG::libelfin INTERFACE libelfin::libelfin)
	endif()
endif()

# cxxopts
# ----
find_package(cxxopts CONFIG REQUIRED)
if(NOT TARGET CONAN_PKG::cxxopts)
	add_library(CONAN_PKG::cxxopts INTERFACE IMPORTED GLOBAL)
	target_link_libraries(CONAN_PKG::cxxopts INTERFACE cxxopts::cxxopts)
endif()
