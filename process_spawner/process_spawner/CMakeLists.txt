add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmycommon libmycommon)
find_library(libmypspawner libmypspawner)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmycommon)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmypspawner)
