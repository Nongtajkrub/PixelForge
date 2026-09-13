add_subdirectory(c)
add_subdirectory(cplusplus)

add_library(Core INTERFACE)

target_link_libraries(Core
	INTERFACE
		CoreC
		CoreCplusplus
)
