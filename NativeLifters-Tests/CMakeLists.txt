cmake_minimum_required(VERSION 3.16)

project(NativeLifters-Tests)

add_executable(${PROJECT_NAME}
   main.cpp
   emulator/emulator.cpp
   emulator/rwx_allocator.cpp
)  

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:0x10000000")
endif()

target_link_libraries(${PROJECT_NAME} NativeLifters-Core)