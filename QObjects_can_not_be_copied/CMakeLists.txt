cmake_minimum_required(VERSION 3.19)
project(QObjects_can_not_be_copied LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(QObjects_can_not_be_copied
    main.cpp
    cat.h cat.cpp
)

target_link_libraries(QObjects_can_not_be_copied
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS QObjects_can_not_be_copied
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
