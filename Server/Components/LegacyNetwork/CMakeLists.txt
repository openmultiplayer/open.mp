get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

add_definitions(
	-DTTMATH_NOASM
)

target_link_libraries(${ProjectId} PRIVATE
	raknet
	ttmath
)