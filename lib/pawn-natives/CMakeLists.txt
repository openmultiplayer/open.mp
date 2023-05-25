cmake_minimum_required(VERSION 3.1)

if (NOT MAIN_RUN)
		project(dependencies)

		include(ExternalProject)

		ExternalProject_Add(samp-plugin-sdk
				PREFIX "samp-plugin-sdk"
				GIT_REPOSITORY "https://github.com/Y-Less/samp-plugin-sdk.git"
				GIT_TAG "master"
				UPDATE_COMMAND ""
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
		)

		if (NOT WIN32)
				ExternalProject_Add(sampgdk
						PREFIX "sampgdk"
						GIT_REPOSITORY "https://github.com/Zeex/sampgdk.git"
						GIT_TAG "master"
						UPDATE_COMMAND ""
						BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
						INSTALL_COMMAND ""
						CMAKE_ARGS "-DSAMP_SDK_ROOT=${CMAKE_SOURCE_DIR}/samp-plugin-sdk/src/samp-plugin-sdk"
				)
		endif ()

		ExternalProject_Add(cmake-modules
				PREFIX "cmake-modules"
				GIT_REPOSITORY "https://github.com/Zeex/cmake-modules.git"
				GIT_TAG "master"
				UPDATE_COMMAND ""
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
		)

		ExternalProject_Add(subhook
				PREFIX "subhook"
				GIT_REPOSITORY "https://github.com/Zeex/subhook.git"
				GIT_TAG "master"
				UPDATE_COMMAND ""
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
		)

		if (WIN32)
				# Windows seems to go top-to-bottom.  Linux the other way...
				ExternalProject_Add(sampgdk
						PREFIX "sampgdk"
						GIT_REPOSITORY "https://github.com/Zeex/sampgdk.git"
						GIT_TAG "master"
						UPDATE_COMMAND ""
						BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
						INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target Install
						CMAKE_ARGS "-DSAMP_SDK_ROOT=${CMAKE_SOURCE_DIR}/samp-plugin-sdk/src/samp-plugin-sdk"
				)
		endif ()

		add_custom_command(TARGET sampgdk POST_BUILD
				COMMAND "${CMAKE_COMMAND}" ${CMAKE_SOURCE_DIR} -DMAIN_RUN=TRUE
				COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_SOURCE_DIR}
				COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_SOURCE_DIR} --config Release
		)
else ()
		project(pawn-natives)

		# Now that we have them, use them.
		list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake-modules/src/cmake-modules)
		include(AMXConfig)
		include(AddSAMPPlugin)

		include_directories(
				samp-plugin-sdk/src/samp-plugin-sdk/
				samp-plugin-sdk/src/samp-plugin-sdk/amx
				subhook/src/
				sampgdk/src/sampgdk/include/
				sampgdk/src/sampgdk-build/include/
		)

		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_definitions(-DSUBHOOK_IMPLEMENTATION)
		add_definitions(-DHAVE_STDINT_H=1)

		add_samp_plugin(pawn-natives
				test.cpp
				subhook/src/subhook/subhook.c
				samp-plugin-sdk/src/samp-plugin-sdk/amxplugin.cpp
				samp-plugin-sdk/src/samp-plugin-sdk/amxplugin2.cpp
		)

		if (WIN32)
				target_link_libraries(pawn-natives
						${CMAKE_SOURCE_DIR}/sampgdk/src/sampgdk-build/Release/sampgdk4.lib
				)

				set_target_properties(pawn-natives PROPERTIES LINKER_LANGUAGE CXX)
		else ()
				target_link_libraries(pawn-natives
						${CMAKE_SOURCE_DIR}/sampgdk/src/sampgdk-build/libsampgdk.so
				)

				set(CMAKE_CXX_FLAGS "-std=gnu++0x")
				if(TARGET_BUILD_ARCH MATCHES "(i[3-6]86|x86)" AND NOT TARGET_BUILD_ARCH MATCHES "(x64|x86_64|amd64)")
					set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
				endif()
		endif ()
endif ()



