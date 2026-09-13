# Core/cplusplus/CMakeLists.txt

add_library(CoreCplusplus STATIC)

target_sources(CoreCplusplus
    PRIVATE
        src/io/byte_io.cpp
        src/io/file_io.cpp
        src/utilities/unique_string.cpp
)

target_include_directories(CoreCplusplus
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(CoreCplusplus
	PRIVATE
		CoreC
)
