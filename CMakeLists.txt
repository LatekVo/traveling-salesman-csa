cmake_minimum_required(VERSION 3.23)
project(tspExec)

set(CMAKE_CXX_STANDARD 14)

find_package(SFML REQUIRED COMPONENTS graphics system window)
include_directories(${SFML_INCLUDE_DIR} ${rsc})

add_executable(tspExec main.cpp)

set(SFML_LIBRARIES sfml-system sfml-graphics sfml-window)

# linking
target_link_libraries(tspExec ${SFML_LIBRARIES})