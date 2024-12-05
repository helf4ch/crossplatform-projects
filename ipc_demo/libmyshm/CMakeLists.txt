project(libmyshm)

add_subdirectory(${PROJECT_NAME}/)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
