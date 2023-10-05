set(RUNTIME_SRC
	runtime.cpp
)

add_library(runtime STATIC ${RUNTIME_SRC})

target_link_libraries(runtime PRIVATE interpreter)

set(RUNTIME_DIR ${PROJECT_ROOT}/runtime)

target_include_directories(runtime PUBLIC
	${RUNTIME_DIR}
	${RUNTIME_DIR}/interpreter
	${PROJECT_ROOT}/common
)


add_subdirectory(interpreter)