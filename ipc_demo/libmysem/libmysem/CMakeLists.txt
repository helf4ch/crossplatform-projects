add_library(${PROJECT_NAME} mysem.cpp)

find_library(libmycommon libmycommon)
find_library(libmyshm libmyshm)

target_link_libraries(${PROJECT_NAME} PRIVATE libmycommon)
target_link_libraries(${PROJECT_NAME} PRIVATE libmyshm)
