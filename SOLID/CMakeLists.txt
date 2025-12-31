cmake_minimum_required(VERSION 3.19)
project(SOLID LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(SOLID
    main.cpp
    tempconverter.h tempconverter.cpp
)

target_link_libraries(SOLID
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS SOLID
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
