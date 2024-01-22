cmake_minimum_required(VERSION 3.12)
project(RC_CAR)

find_package(OpenCV REQUIRED)

add_executable(RC_CAR main2.cpp)
target_link_libraries(RC_CAR ${OpenCV_LIBS})