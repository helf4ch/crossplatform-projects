add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmyshm libmyshm)
find_library(libmythread libmyshm)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmyshm)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmythread)
