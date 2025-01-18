add_executable(${CMAKE_PROJECT_NAME} main.cpp)

find_library(libmyhttp libmyhttp)
find_library(libmyserial libmyserial)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmyhttp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libmyserial)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)

if (WIN32)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ws2_32)
endif()
