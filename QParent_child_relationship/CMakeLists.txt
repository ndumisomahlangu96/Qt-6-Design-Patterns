cmake_minimum_required(VERSION 3.19)
project(QParent_child_relationship LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(QParent_child_relationship
    main.cpp
    test.h test.cpp
)

target_link_libraries(QParent_child_relationship
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS QParent_child_relationship
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
