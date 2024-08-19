cmake_minimum_required(VERSION 3.12)
project(RC_CAR LANGUAGES CXX)

# Ensure a build type is set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set general C++ flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# Set debug-specific flags
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Override the optimization level for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

# Find necessary packages
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)

# Define the executable and link libraries
add_executable(RC_CAR main.cpp)
target_include_directories(RC_CAR PUBLIC
    "${OpenCV_INCLUDE_DIRS}"
    "${GPIOD_INCLUDE_DIRS}")
target_link_libraries(RC_CAR PUBLIC
    ${OpenCV_LIBS}
    ${GPIOD_LIBRARIES})

# Print the release flags to verify
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")