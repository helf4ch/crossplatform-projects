add_library(${PROJECT_NAME} mysem.cpp)

find_library(libmyshm libmyshm)

target_link_libraries(${PROJECT_NAME} PRIVATE libmyshm)
