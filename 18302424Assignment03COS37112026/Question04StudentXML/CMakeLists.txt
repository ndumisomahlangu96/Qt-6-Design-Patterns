cmake_minimum_required(VERSION 3.19)
project(Question04StudentXML LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Xml)

qt_standard_project_setup()

qt_add_executable(Question04StudentXML
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    getstudentinformation.h getstudentinformation.cpp
    student.h student.cpp
    studentlist.h studentlist.cpp
)

target_link_libraries(Question04StudentXML
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Xml

)

install(TARGETS Question04StudentXML
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Question04StudentXML
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
