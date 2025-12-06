cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(argvx LANGUAGES CXX)
add_library(argvx INTERFACE)
target_include_directories(argvx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (ARGVX_BUILD_TEST)
  project(argvx-test LANGUAGES CXX)
  add_executable(argvx-test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
  target_link_libraries(argvx-test argvx)
endif()
