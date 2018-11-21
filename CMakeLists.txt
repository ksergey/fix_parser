cmake_minimum_required(VERSION 3.0)

project(fix_parser)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
    set(BUILD_ALL OFF)
else()
    set(BUILD_ALL ON)
endif()

option(FixParser_BUILD_TESTS "Build tests" ${BUILD_ALL})

include(ExternalProject)

set(EXT_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ext")

ExternalProject_Add(LibXml2-build
    URL "ftp://xmlsoft.org/libxml2/libxml2-2.8.0.tar.gz"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./configure --prefix=${EXT_INSTALL_PREFIX} --disable-shared --enable-static
        --without-debug
        --without-iconv
        --without-lzma
        --without-python
        --without-threads
        --without-zlib
)


include(CheckFunctionExists)

check_function_exists(pow LIBM_IS_OPT)
if(NOT LIBM_IS_OPT)
    find_library(libm_opt NAMES m)
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${libm_opt})
    check_function_exists(pow RESULT)
    if(NOT RESULT)
        message(FATAL_ERROR "libm not found")
    endif()
endif()

add_library(LibXml2-static INTERFACE)
add_dependencies(LibXml2-static LibXml2-build)
target_link_libraries(LibXml2-static INTERFACE ${EXT_INSTALL_PREFIX}/lib/libxml2.a ${libm_opt})
target_include_directories(LibXml2-static INTERFACE ${EXT_INSTALL_PREFIX}/include/libxml2)

set(target fix_parser)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

add_library(${target} ${sources} ${headers})
target_link_libraries(${target}
    PUBLIC
        LibXml2-static
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${target} PRIVATE -std=gnu99 -Wall)

if (FixParser_BUILD_TESTS)
   add_subdirectory(perf_test)
   add_subdirectory(test)
endif()
