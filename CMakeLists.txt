cmake_minimum_required(VERSION 3.12)
project(RC_CAR)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
#find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
#find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
add_executable(RC_CAR main2.cpp)
target_link_libraries(RC_CAR ${OpenCV_LIBS})

include_directories(${GPIOD_LIBRARIES})

target_link_libraries(RC_CAR ${GPIOD_LIBRARIES})
