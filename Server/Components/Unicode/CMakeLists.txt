get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_server_component(${ProjectId})

if (STATIC_STDCXX)
	# Remove any possible symbols from ICU, lower GLIBCXX version to 3.4.11
	target_compile_options(${ProjectId} PRIVATE -fno-exceptions -fno-rtti)
endif()

target_link_libraries(${ProjectId} PRIVATE
	CONAN_PKG::icu
)
