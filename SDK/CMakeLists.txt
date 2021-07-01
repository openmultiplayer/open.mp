set(PROJECT SDK)
add_library(SDK INTERFACE)

target_link_libraries(SDK INTERFACE
	CONAN_PKG::glm
	CONAN_PKG::nlohmann_json
)

target_include_directories(SDK INTERFACE .)

file(GLOB_RECURSE sdk_source_list "*.hpp")

set_property(TARGET SDK PROPERTY SOURCES ${sdk_source_list})

GroupSourcesByFolder(SDK)

set_property(TARGET SDK PROPERTY OUTPUT_NAME omp-sdk)
set_property(TARGET SDK PROPERTY FOLDER "SDK")
