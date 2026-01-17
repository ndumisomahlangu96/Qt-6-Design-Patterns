cmake_minimum_required(VERSION 3.19)
project(WriteXML LANGUAGES CXX)

# 1. Add "Xml" to the required components
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Xml)

qt_standard_project_setup()

qt_add_executable(WriteXML
    main.cpp
)

# 2. Link the "Qt::Xml" library to your target
target_link_libraries(WriteXML
    PRIVATE
        Qt::Core
        Qt::Xml
)

include(GNUInstallDirs)

install(TARGETS WriteXML
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
