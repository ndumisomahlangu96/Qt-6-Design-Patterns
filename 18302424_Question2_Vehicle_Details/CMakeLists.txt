cmake_minimum_required(VERSION 3.19)
project(18302424_Question2_Vehicle_Details LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(18302424_Question2_Vehicle_Details
    main.cpp
    vehicledetails.h vehicledetails.cpp
    passengervehicles.h passengervehicles.cpp
    transportvehicles.h transportvehicles.cpp
    filewriter.h filewriter.cpp


)

target_link_libraries(18302424_Question2_Vehicle_Details
    PRIVATE
        Qt::Core
)

install(TARGETS 18302424_Question2_Vehicle_Details
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
