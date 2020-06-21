project(NativeLifters-Core)

add_library(${PROJECT_NAME} STATIC
	amd64/amd64.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC includes)

target_link_libraries(${PROJECT_NAME} VTIL)