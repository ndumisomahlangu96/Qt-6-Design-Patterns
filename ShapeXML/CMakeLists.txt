cmake_minimum_required(VERSION 3.19)
project(ShapeXML LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Xml)

qt_standard_project_setup()

qt_add_executable(ShapeXML
    main.cpp
)

target_link_libraries(ShapeXML
    PRIVATE
        Qt::Core
        Qt::Xml

)

install(TARGETS ShapeXML
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
