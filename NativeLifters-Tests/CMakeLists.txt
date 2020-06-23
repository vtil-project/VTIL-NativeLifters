project(NativeLifters-Tests)

add_executable(${PROJECT_NAME}
    main.cpp
    emulator/emulator.cpp
    emulator/rwx_allocator.cpp
)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:0x10000000 /WHOLEARCHIVE:NativeLifters-Core")
endif()

target_link_libraries(${PROJECT_NAME} NativeLifters-Core)