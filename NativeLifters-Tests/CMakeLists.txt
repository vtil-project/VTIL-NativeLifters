project(NativeLifters-Tests)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp *.hpp *.h)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /STACK:0x10000000)
endif()

target_link_libraries(${PROJECT_NAME} NativeLifters-Core)