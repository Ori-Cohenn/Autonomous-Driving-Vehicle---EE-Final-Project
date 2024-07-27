cmake_minimum_required(VERSION 3.12)
project(RC_CAR LANGUAGES CXX)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
add_executable(RC_CAR main.cpp)
target_include_directories(RC_CAR PUBLIC
    "${OpenCV_INCLUDE_DIRS}"
    "${GPIOD_INCLUDE_DIRS}")
target_link_libraries(RC_CAR PUBLIC
    ${OpenCV_LIBS}
    ${GPIOD_LIBRARIES})