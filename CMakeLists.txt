cmake_minimum_required(VERSION 2.8)
project(mocker)

set(CMAKE_VERBOSE_MAKEFILE ON)
# enable c++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
        mocker/log.cpp
    )

add_library(mocker SHARED ${LIB_SRC})
#add_library(mocker_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(mocker_static PROPERTIES OUTPUT_NAME "mocker")

# add test
add_executable(test tests/test.cpp)
add_dependencies(test mocker)
target_link_libraries(test mocker)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
