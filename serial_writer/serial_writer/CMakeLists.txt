add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmyprint libmyprint)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC libmyprint)
