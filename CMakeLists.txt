cmake_minimum_required(VERSION 3.0)

project(fix_parser)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

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

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(XML2 REQUIRED libxml-2.0 libxml2 libxml>=2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")

aux_source_directory(src LIB_SOURCES)
aux_source_directory(src/lin LIB_SOURCES)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
        ${XML2_STATIC_LIBRARIES}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${XML2_STATIC_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (FixParser_BUILD_TESTS)
   add_subdirectory(perf_test)
   add_subdirectory(test)
endif()
