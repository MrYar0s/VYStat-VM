add_custom_target(interpreter_cts_tests)
add_dependencies(tests interpreter_cts_tests)

function(shrimp_cts_test test_name)
	set(TEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
	set(TEST_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.shr)
	file(MAKE_DIRECTORY ${TEST_BUILD_DIR})

	add_custom_target(compile_cts_${test_name}
		COMMAND cd ${TEST_BUILD_DIR} && ${PROJECT_BINARY_DIR}/assembler/assembler
		--in ${TEST_SOURCE_PATH}
		--out ${TEST_BUILD_DIR}/${test_name}.imp
		DEPENDS assembler ${TEST_SOURCE_PATH}
	)
	
	add_custom_target(run_cts_${test_name}
		COMMAND ${PROJECT_BINARY_DIR}/shrimp/shrimp
		--in ${TEST_BUILD_DIR}/${test_name}.imp
		DEPENDS compile_cts_${test_name} shrimp
	)

	add_dependencies(interpreter_cts_tests run_cts_${test_name})
endfunction()

set(TEST_FILES
	"jump"
	"jump_eq"
	"jump_ll"
	"jump_gg"
	"lda_imm_i32"
	"mov_imm_i32"
	"add_i32"
	"div_i32"
	"mul_i32"
	"sub_i32"
	"mov"
	"lda"
	"sta"
	"lda_imm_f"
	"mov_imm_f"
	"add_f"
	"div_f"
	"mul_f"
	"sub_f"
)

foreach(file IN LISTS TEST_FILES)
	shrimp_cts_test(${file})
endforeach()