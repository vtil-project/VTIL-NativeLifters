# Define project
project(VTIL-NativeLifters)

# For portability on non-MSVC compilers, add some compile options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wno-narrowing)

    # Clang-only flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Use libc++ if compiling C++
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++")
    endif()

    # GCC-only flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wvolatile)
        add_compile_options(-fpermissive)
    endif()
endif()



if(MSVC)
    # Supress CRT security warnings
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
    # -fpermissive MSVC equivalent
    add_compile_options(/permissive-)
endif()

# Add VTIL-Core as a dependency
add_subdirectory(Dependencies/VTIL-Core)

# Set default C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Include subprojects
add_subdirectory(NativeLifters-X86)