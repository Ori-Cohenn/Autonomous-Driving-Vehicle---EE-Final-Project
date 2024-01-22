cmake_minimum_required(VERSION 3.12)
project(RC_CAR)

find_package(OpenCV REQUIRED)
find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)
add_executable(RC_CAR main2.cpp)
target_link_libraries(RC_CAR ${OpenCV_LIBS})


# Include directories for WiringPi
include_directories(${WIRINGPI_INCLUDE_DIRS})

# Link against WiringPi
target_link_libraries(RC_CAR ${WIRINGPI_LIBRARIES})

