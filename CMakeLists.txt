cmake_minimum_required(VERSION 3.0)

project(fix_parser)

get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
    set(BUILD_ALL OFF)
else()
    set(BUILD_ALL ON)
endif()

option(FixParser_BUILD_TESTS "Build tests" ${BUILD_ALL})

if (WIN32)
   set(LIBXML2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/deps/libxml2/include)
   set(LIBXML2_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/deps/libxml2/lib)
else(WIN32)
   find_package(LibXml2 REQUIRED)
endif(WIN32)

find_package(Threads REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR} ./include ./src)
link_directories(${LIBXML2_LIBRARY_DIR})

if (WIN32)
   add_definitions(-D_CRT_SECURE_NO_WARNINGS)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
else(WIN32)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -fPIC")
endif(WIN32)

aux_source_directory(src LIB_SOURCES)
if (WIN32)
   aux_source_directory(src/win LIB_SOURCES)
else()
   aux_source_directory(src/lin LIB_SOURCES)
endif()

if (WIN32)
   set_source_files_properties(${LIB_SOURCES} PROPERTIES LANGUAGE CXX)
endif()

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
add_library(${PROJECT_NAME}_s STATIC ${LIB_SOURCES})

if (WIN32)
   target_link_libraries(${PROJECT_NAME} libxml2)
   target_link_libraries(${PROJECT_NAME}_s libxml2)
   set_target_properties(${PROJECT_NAME}_s PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_s)
else()
   target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})
   target_link_libraries(${PROJECT_NAME}_s ${LIBXML2_LIBRARIES})
   set_target_properties(${PROJECT_NAME}_s PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME}_s ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME}_s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME}_s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (FixParser_BUILD_TESTS)
   add_subdirectory(perf_test)
   add_subdirectory(test)
endif()
