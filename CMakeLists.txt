# Define project
project(VTIL-NativeLifters)

# Add VTIL-Core as a dependency
add_subdirectory(Dependencies/VTIL-Core)

# Set default C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Include subprojects
add_subdirectory(NativeLifters-Core)

# Tests
if(VTIL_BUILD_TESTS)
    add_subdirectory(NativeLifters-Tests)
endif()