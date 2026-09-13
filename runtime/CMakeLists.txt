add_executable(Runtime)

target_sources(Runtime
	PRIVATE
		src/main.cpp
)

target_link_libraries(Runtime
	PRIVATE
		Engine
)

set_target_properties(Runtime PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
