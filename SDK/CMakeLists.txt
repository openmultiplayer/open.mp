set(PROJECT SDK)
add_library(SDK INTERFACE)

target_link_libraries(SDK INTERFACE
	CONAN_PKG::glm
	CONAN_PKG::nlohmann_json
)

file(GLOB sdk_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

set_property(TARGET SDK PROPERTY SOURCES ${sdk_source_list})

GroupSourcesByFolder(SDK)

set_property(TARGET SDK PROPERTY OUTPUT_NAME omp-sdk)
set_property(TARGET SDK PROPERTY FOLDER "SDK")
