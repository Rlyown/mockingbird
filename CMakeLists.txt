cmake_minimum_required(VERSION 2.8)
project(mocker)

set(CMAKE_VERBOSE_MAKEFILE ON)
# enable c++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)

set(LIB_SRC
        mocker/log.cpp
        mocker/util.cpp
        mocker/config.cpp)

add_library(mocker SHARED ${LIB_SRC})
#add_library(mocker_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(mocker_static PROPERTIES OUTPUT_NAME "mocker")

# add test
add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log mocker)
target_link_libraries(test_log mocker yaml-cpp)

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config mocker)
target_link_libraries(test_config mocker yaml-cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
