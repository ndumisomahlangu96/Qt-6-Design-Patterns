cmake_minimum_required(VERSION 3.19)
project(Open_Closed_Principle LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(Open_Closed_Principle
    main.cpp
)

target_link_libraries(Open_Closed_Principle
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS Open_Closed_Principle
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
