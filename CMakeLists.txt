cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(shrimp)
enable_language(CXX)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR})
set(PROJECT_BINARY_ROOT ${PROJECT_BINARY_DIR})
set(PROJECT_THIRD_PARTY ${PROJECT_ROOT}/third-party)
set(PROJECT_SCRIPTS ${PROJECT_ROOT}/scripts)

find_package(Python3 REQUIRED)

execute_process(
	COMMAND ${Python3_EXECUTABLE} ${PROJECT_SCRIPTS}/install_third_party.py
		${PROJECT_SCRIPTS}/third_party_list.txt ${PROJECT_THIRD_PARTY}
)

if (ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        --format-style=file;
        --header-filter=*.h
    )
endif()

add_custom_target(
	run-clang-format
	COMMENT Run clang-format
	COMMAND ${Python3_EXECUTABLE} ${PROJECT_SCRIPTS}/run_clang_format.py ${PROJECT_ROOT}
)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/runtime
)

target_link_libraries(${PROJECT_NAME} PRIVATE runtime)

add_subdirectory(third-party/googletest)

add_subdirectory(common)
add_subdirectory(runtime)
add_subdirectory(assembler)
