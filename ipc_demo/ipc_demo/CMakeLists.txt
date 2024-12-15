add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmycommon libmycommon)
find_library(libmyshm libmyshm)
find_library(libmysem libmysem)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmycommon)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmyshm)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmysem)
