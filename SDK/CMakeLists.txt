set(PROJECT SDK)
add_library(SDK INTERFACE)

target_link_libraries(SDK INTERFACE
	CONAN_PKG::glm
	CONAN_PKG::abseil
)

target_compile_definitions(SDK INTERFACE
	GLM_FORCE_SSE2
	GLM_FORCE_QUAT_DATA_WXYZ
	NOMINMAX
)

target_include_directories(SDK INTERFACE .)

file(GLOB_RECURSE sdk_source_list "*.hpp")

set_property(TARGET SDK PROPERTY SOURCES ${sdk_source_list})

GroupSourcesByFolder(SDK)

set_property(TARGET SDK PROPERTY OUTPUT_NAME omp-sdk)
set_property(TARGET SDK PROPERTY FOLDER "SDK")
