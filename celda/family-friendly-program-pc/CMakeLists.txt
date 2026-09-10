cmake_minimum_required(VERSION 3.9)
project(celda_carga VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(CSerialPort REQUIRED)

include_directories(${CSerialPort_INCLUDE_DIR})

add_executable(celda_carga
	src/main.cpp
	src/main_window.cpp
	src/connection.cpp
	src/painter.cpp
	src/choosing.cpp
)

target_link_libraries(celda_carga PRIVATE Qt5::Widgets)
target_link_libraries(celda_carga PRIVATE ${CSerialPort_LIBRARY})

