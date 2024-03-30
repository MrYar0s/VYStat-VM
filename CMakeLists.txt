# Describe project build

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(shrimp VERSION 0.1 LANGUAGES CXX)

find_package(Python3 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(PROJECT_THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/third-party)
set(PROJECT_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

include(isa/isa.cmake)

# Add third party
execute_process(
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SCRIPTS}/install_third_party.py
        ${PROJECT_SCRIPTS}/third_party_list.txt ${PROJECT_THIRD_PARTY}
)

add_custom_target(
    run-clang-format
    COMMENT Run clang-format
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SCRIPTS}/run_clang_format.py ${PROJECT_SOURCE_DIR}
)

add_custom_target(
	run-clang-tidy
	COMMENT Run clang-tidy
	COMMAND ${Python3_EXECUTABLE} ${PROJECT_SCRIPTS}/run_clang_tidy.py ${PROJECT_SOURCE_DIR}
	DEPENDS all
)

# Add GTest
add_subdirectory(third-party/googletest)

# Add CLI11
add_subdirectory(third-party/CLI11)

# Add libraries
add_subdirectory(shrimpfile)
add_subdirectory(common)
add_subdirectory(runtime)
add_subdirectory(frontend)

# Add assembler executable
add_subdirectory(assembler)

# Add VM executable
add_subdirectory(shrimp)

add_subdirectory(tests)