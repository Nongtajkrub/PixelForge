add_library(CoreC STATIC)

target_sources(CoreC
    PRIVATE
        src/container/array.c
        src/container/block.c
        src/container/vec.c
        src/io/vec_stream.c
)

target_include_directories(CoreC
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
