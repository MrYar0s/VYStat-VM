# Path to python library for instructions description processing
set(ISA_PYTHONPATH ${PROJECT_SOURCE_DIR}/isa)
# Path to yaml file with instructions description
set(ISA_YAML ${PROJECT_SOURCE_DIR}/isa/isa.yaml)

# Add file generation based on instructions description.
# Provided python script is run with ${ISA_YAML} and ${output} args.
function(shrimp_add_instr_gen script output)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${ISA_PYTHONPATH} python3 ${script} ${ISA_YAML} ${output}
        DEPENDS ${ISA_PYTHONPATH} ${ISA_YAML} ${script}
    )
endfunction(shrimp_add_instr_gen)

# Call shrimp_add_instr_gen(${script} ${output}) and make ${dependent_target} dependent on ${output}
# Custom target ${custom_target} is created to add dependency
function(shrimp_add_instr_dep_gen script output dependent_target custom_target)
    shrimp_add_instr_gen(${script} ${output})
    add_custom_target(${custom_target} DEPENDS ${output})
    add_dependencies(${dependent_target} ${custom_target})
endfunction(shrimp_add_instr_dep_gen)
