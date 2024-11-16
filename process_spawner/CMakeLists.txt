cmake_minimum_required(VERSION 3.15...3.31)

project(process_spawner VERSION 1.0)

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

add_subdirectory(${PROJECT_NAME}/)
