get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

# SQLite 3
# ---
add_conan_lib(CONAN_PKG::sqlite3 sqlite3/3.36.0)

target_link_libraries(${ProjectId} PRIVATE
	CONAN_PKG::sqlite3
)
