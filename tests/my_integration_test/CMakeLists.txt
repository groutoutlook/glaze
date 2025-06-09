project(my_integration_test)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

FetchContent_Declare(
    choc_stb
    GIT_REPOSITORY https://github.com/groutoutlook/choc_stb
    GIT_TAG        origin/main
)
FetchContent_MakeAvailable(choc_stb)

target_link_libraries(${PROJECT_NAME} PRIVATE glz_test_common choc_stb)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    GLZ_TEST_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_code_coverage(${PROJECT_NAME} AUTO ALL)
