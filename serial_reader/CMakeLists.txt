cmake_minimum_required(VERSION 3.15...3.31)

project(serial_reader VERSION 1.0)

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

find_package(SQLite3 REQUIRED)

add_subdirectory(json/)
add_subdirectory(libmycommon/)
add_subdirectory(libmyhttp/)
add_subdirectory(libmyserial/)
add_subdirectory(${PROJECT_NAME}/)
