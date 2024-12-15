add_library(${PROJECT_NAME} myshm.hpp)

find_library(libmycommon libmycommon)

target_link_libraries(${PROJECT_NAME} PRIVATE libmycommon)
