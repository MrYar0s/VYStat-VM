# Describe project build

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(shrimp VERSION 0.1 LANGUAGES CXX)

find_package(Python3 REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(PROJECT_THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/third-party)
set(PROJECT_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

# Path to yaml file with instructions description
set(INSTR_YAML ${CMAKE_CURRENT_SOURCE_DIR}/instr.yaml)
# Path to python library for instructions description processing
set(INSTR_PYTHONPATH ${CMAKE_CURRENT_SOURCE_DIR})

# Add file generation based on instructions description.
# Provided python script is run with ${INSTR_YAML} and ${output} args.
# ${INSTR_PY} is added to PYTHONPATH env variable
function(shrimp_add_instr_gen script output)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${INSTR_PYTHONPATH} python3 ${script} ${INSTR_YAML} ${output}
        DEPENDS ${INSTR_PY} ${INSTR_YAML} ${script}
    )
endfunction(shrimp_add_instr_gen)

# Call shrimp_add_instr_gen(${script} ${output}) and make ${dependent_target} dependent on ${output}
# Custom target ${custom_target} is created to add dependency
function(shrimp_add_instr_dep_gen script output dependent_target custom_target)
    shrimp_add_instr_gen(${script} ${output})
    add_custom_target(${custom_target} DEPENDS ${output})
    add_dependencies(${dependent_target} ${custom_target})
endfunction(shrimp_add_instr_dep_gen)

# Add third party
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

# Add GTest
add_subdirectory(third-party/googletest)

# Add CLI11
add_subdirectory(third-party/CLI11)

# Add libraries
add_subdirectory(common)
add_subdirectory(runtime)

# Add assembler executable
add_subdirectory(assembler)

# Add VM executable
add_subdirectory(shrimp)
