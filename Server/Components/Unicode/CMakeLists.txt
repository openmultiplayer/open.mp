get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

target_compile_options(${ProjectId} PRIVATE -fno-exceptions)

target_link_libraries(${ProjectId} PRIVATE
	CONAN_PKG::icu
)
