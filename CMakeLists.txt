cmake_minimum_required(VERSION 3.16)

# Define project
project(VTIL-NativeLifters)

# Project options
option(NATIVELIFTERS_BUILD_TESTS "Build tests" OFF)

# Add VTIL-Core as a dependency
add_subdirectory(Dependencies/VTIL-Core)

# Include subprojects
add_subdirectory(NativeLifters-Core)

# Tests
if(NATIVELIFTERS_BUILD_TESTS)
    add_subdirectory(NativeLifters-Tests)
endif()