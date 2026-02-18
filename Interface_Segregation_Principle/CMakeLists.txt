cmake_minimum_required(VERSION 3.19)
project(Interface_Segregation_Principle LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(Interface_Segregation_Principle
    main.cpp

    interfaces/iFlying.h
    interfaces/iMachine.h
    airplane.h airplane.cpp
    bird.h bird.cpp


)

target_link_libraries(Interface_Segregation_Principle
    PRIVATE
        Qt::Core
)
target_link_libraries(Interface_Segregation_Principle PRIVATE Qt6::Core)
target_link_libraries(Interface_Segregation_Principle PRIVATE Qt6::Core)
target_link_libraries(Interface_Segregation_Principle PRIVATE Qt6::Core)

include(GNUInstallDirs)

install(TARGETS Interface_Segregation_Principle
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
