project(libmyshm)

add_subdirectory(${PROJECT_NAME}/)

set_target_properties(libmyshm PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
