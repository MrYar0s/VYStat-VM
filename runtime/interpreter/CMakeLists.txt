add_library(interpreter STATIC interpreter.cpp)

target_include_directories(interpreter PUBLIC
	${RUNTIME_DIR}
	${RUNTIME_DIR}/interpreter
	${PROJECT_SOURCE_DIR}/common
)
