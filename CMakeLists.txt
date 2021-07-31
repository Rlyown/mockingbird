cmake_minimum_required(VERSION 2.8)
project(mocker)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
# enable c++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-dev -Wno-builtin-macro-redefined")

include_directories(.)

set(LIB_SRC
        mocker/log.cpp
        mocker/util.cpp
        mocker/config.cpp
        mocker/thread.cpp mocker/mutex.cpp)

add_library(mocker SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(mocker)  # __FILE__
#add_library(mocker_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(mocker_static PROPERTIES OUTPUT_NAME "mocker")

# set libs to link
set(LIB_LIB
        mocker
        pthread
        yaml-cpp)

# add test
mocker_build_target(test_log tests/test_log.cpp)
mocker_build_target(test_config tests/test_config.cpp)
mocker_build_target(test_thread tests/test_thread.cpp)
mocker_build_target(test_util tests/test_util.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
