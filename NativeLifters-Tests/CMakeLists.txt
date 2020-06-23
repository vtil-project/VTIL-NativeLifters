project(NativeLifters-Tests)

add_executable(${PROJECT_NAME}
    main.cpp
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:0x10000000 /WHOLEARCHIVE:NativeLifters-Core")
target_link_libraries(${PROJECT_NAME} NativeLifters-Core)