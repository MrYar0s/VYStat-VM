set(FRONTEND_SRC
    lang2shrimp.cpp
    lexer.cpp
    main.cpp
    parser.cpp
)

add_executable(lang2shrimp ${FRONTEND_SRC})

target_link_libraries(lang2shrimp PRIVATE shrimpfile CLI11)

target_include_directories(lang2shrimp
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/assembler/include
)
