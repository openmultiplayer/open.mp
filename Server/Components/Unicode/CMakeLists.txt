get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

target_compile_definitions(${ProjectId} PRIVATE -DU_STATIC_IMPLEMENTATION)

target_link_libraries(${ProjectId} PRIVATE
	CONAN_PKG::icu
)
