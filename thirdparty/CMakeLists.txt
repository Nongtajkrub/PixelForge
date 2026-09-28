add_library(Thirdparty INTERFACE)

target_include_directories(Thirdparty
	INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>
)
