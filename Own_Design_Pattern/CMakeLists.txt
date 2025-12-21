cmake_minimum_required(VERSION 3.19)
project(Own_Design_Pattern LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(Own_Design_Pattern
    main.cpp
)

target_link_libraries(Own_Design_Pattern
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS Own_Design_Pattern
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
