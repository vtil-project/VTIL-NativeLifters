# Require at least CMake version 3.15 or later for FetchContent
cmake_minimum_required(VERSION 3.14.5)

# Define project
project(VTIL-NativeLifters)

# Detect if VTIL-Core is compiled as the root project
set(PROJECT_IS_TOP_LEVEL OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL ON)

    # Enable solution folder support
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Project options
option(NATIVELIFTERS_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})

# Add dependencies
set(CMAKE_FOLDER "VTIL-NativeLifters/Dependencies")
add_subdirectory(Dependencies)

# Include subprojects
set(CMAKE_FOLDER "VTIL-NativeLifters")
add_subdirectory(NativeLifters-Core)

# Tests
if(NATIVELIFTERS_BUILD_TESTS)
    add_subdirectory(NativeLifters-Tests)
    enable_testing()
    add_test(NAME NativeLifters-Tests COMMAND "$<TARGET_FILE:NativeLifters-Tests>")
endif()