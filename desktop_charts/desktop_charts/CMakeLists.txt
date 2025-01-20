qt_add_executable(${CMAKE_PROJECT_NAME}
    main.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core 
    Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Charts)
