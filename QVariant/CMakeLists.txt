cmake_minimum_required(VERSION 3.19)
project(QVariant LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(QVariant
    main.cpp
    myclass.h myclass.cpp
)

target_link_libraries(QVariant
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS QVariant
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
