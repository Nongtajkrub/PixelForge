add_library(CoreC STATIC)

target_sources(CoreC
    PRIVATE
		src/utilities/primitive_view.c

		src/allocator/bump_arena.c
)

target_include_directories(CoreC
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
