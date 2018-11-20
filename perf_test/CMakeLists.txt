cmake_minimum_required(VERSION 2.6)

project(perf_test)

aux_source_directory(. PERF_TEST_SOURCES)

add_executable(${PROJECT_NAME} ${PERF_TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} fix_parser)
