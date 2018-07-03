cmake_minimum_required(VERSION 3.0)

project(test)

include(ExternalProject)

ExternalProject_Add(
    googletest
    GIT_REPOSITORY https://github.com/dmitryme/gtest.git
    TIMEOUT 10
    # Disable install step
    INSTALL_COMMAND ""
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Debug -Dgtest_force_shared_crt=ON
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

ExternalProject_Get_Property(googletest SOURCE_DIR)
ExternalProject_Get_Property(googletest BINARY_DIR)

include_directories(${SOURCE_DIR}/include)
link_directories(${BINARY_DIR})

set(TEST_SOURCES fix_field_tests.cc fix_msg_tests.cc fix_parser_priv_tests.cc
    fix_parser_tests.cc fix_protocol_tests.cc fix_utils_tests.cc main.cc)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} gtest fix_parser_s)

add_executable(fix_parser_issue1_tests fix_parser_issue1_tests.cc)
target_link_libraries(fix_parser_issue1_tests fix_parser_s)

file(COPY test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../fix_descr DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
