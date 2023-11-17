add_custom_target(e2e_tests)
add_dependencies(tests e2e_tests)

function(shrimp_e2e_test test_name)
	set(TEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
	set(TEST_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.shr)
	file(MAKE_DIRECTORY ${TEST_BUILD_DIR})

	add_custom_command(COMMAND cd ${TEST_BUILD_DIR} && ${PROJECT_BINARY_DIR}/assembler/assembler < ${TEST_SOURCE_PATH}
		OUTPUT ${TEST_BUILD_DIR}/${test_name}.imp
		DEPENDS assembler
	)

	add_custom_target(run_e2e_${test_name}
		COMMAND ${PROJECT_BINARY_DIR}/shrimp/shrimp ${TEST_BUILD_DIR}/a.out
		DEPENDS ${TEST_BUILD_DIR}/${test_name}.imp shrimp
	)

	add_dependencies(e2e_tests run_e2e_${test_name})
endfunction()

shrimp_e2e_test(jump)
shrimp_e2e_test(sin_cos)
shrimp_e2e_test(square_eq)
shrimp_e2e_test(trigonometry)