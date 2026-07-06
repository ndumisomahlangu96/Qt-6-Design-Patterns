cmake_minimum_required(VERSION 3.19)
project(18302424Question02COS37112026 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(18302424Question02COS37112026
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    shape.h shape.cpp
)

target_link_libraries(18302424Question02COS37112026
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS 18302424Question02COS37112026
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET 18302424Question02COS37112026
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
