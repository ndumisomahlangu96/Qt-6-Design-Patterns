cmake_minimum_required(VERSION 3.19)
project(Question02AddStudentGUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Question02AddStudentGUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    getstudentinformation.h getstudentinformation.cpp

)

target_link_libraries(Question02AddStudentGUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS Question02AddStudentGUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Question02AddStudentGUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
