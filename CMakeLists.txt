# Require at least CMake version 3.15 or later for FetchContent
cmake_minimum_required(VERSION 3.14.5)

# Define project
project(VTIL-NativeLifters)

# Project options
option(NATIVELIFTERS_BUILD_TESTS "Build tests" OFF)

# Enable solution folder support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add dependencies
set(CMAKE_FOLDER "VTIL-NativeLifters/Dependencies")
add_subdirectory(Dependencies)

# Include subprojects
set(CMAKE_FOLDER "VTIL-NativeLifters")
add_subdirectory(NativeLifters-Core)

# Tests
if(NATIVELIFTERS_BUILD_TESTS)
    add_subdirectory(NativeLifters-Tests)
endif()