project(libmyprint)

add_subdirectory(${PROJECT_NAME}/)

# propagate includes to source dir
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
