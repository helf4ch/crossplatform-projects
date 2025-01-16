add_library(${PROJECT_NAME} myhttp.cpp)

find_library(libmycommon libmycommon)

target_link_libraries(${PROJECT_NAME} PRIVATE libmycommon)
