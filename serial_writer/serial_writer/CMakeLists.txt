add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmyhttp libmyhttp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmyhttp)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
