add_library(CoreC STATIC)

target_sources(CoreC
    PRIVATE
		src/primitive_view.c
)

target_include_directories(CoreC
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
