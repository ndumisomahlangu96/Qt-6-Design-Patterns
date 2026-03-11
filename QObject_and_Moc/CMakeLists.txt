cmake_minimum_required(VERSION 3.19)
project(QObject_and_Moc LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(QObject_and_Moc
    main.cpp
    test.h test.cpp
)

target_link_libraries(QObject_and_Moc
    PRIVATE
        Qt::Core
)

include(GNUInstallDirs)

install(TARGETS QObject_and_Moc
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
