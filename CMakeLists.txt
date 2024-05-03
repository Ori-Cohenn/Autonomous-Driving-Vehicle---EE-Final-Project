cmake_minimum_required(VERSION 3.12)
project(RC_CAR)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
add_executable(RC_CAR main2.cpp)
target_link_libraries(RC_CAR ${OpenCV_LIBS})

include_directories(${GPIOD_LIBRARIES})

target_link_libraries(RC_CAR ${GPIOD_LIBRARIES})
