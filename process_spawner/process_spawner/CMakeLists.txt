add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmypspawner libmypspawner)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC libmypspawner)
