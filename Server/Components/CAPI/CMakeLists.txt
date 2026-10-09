get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

target_compile_definitions(${ProjectId} PRIVATE -DCAPI_COMPONENT_BUILD)
target_compile_options(${ProjectId} PRIVATE -Wno-unused-variable)
target_link_libraries(${ProjectId} PRIVATE OMP-CAPI)

set_target_properties(${ProjectId} PROPERTIES PREFIX "$")
