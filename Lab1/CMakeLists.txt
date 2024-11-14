cmake_minimum_required(VERSION 3.15...3.31)

project(crossplatform_hello_world VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libmyprint/)
add_subdirectory(${PROJECT_NAME}/)
