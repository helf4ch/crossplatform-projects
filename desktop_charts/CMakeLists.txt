cmake_minimum_required(VERSION 3.15...3.31)

project(desktop_charts VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang|GNU")
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} 
      -Wall 
      -Wextra 
      -Wshadow 
      -Werror 
      -Wunnused 
      -pedantic 
      -fsanitize=address"
  )
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

add_subdirectory(${PROJECT_NAME}/)
