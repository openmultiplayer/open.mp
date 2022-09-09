get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

target_link_libraries(${ProjectId} PRIVATE
	pawn-runtime
	CONAN_PKG::ghc-filesystem
)

set_property(TARGET ${ProjectId} PROPERTY CXX_EXTENSIONS ON)

include_directories(${CMAKE_SOURCE_DIR}/lib)
