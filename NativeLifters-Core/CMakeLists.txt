project(NativeLifters-Core)

add_library(${PROJECT_NAME} STATIC
	amd64/amd64.cpp
	amd64/semantic/arithmetic.cpp
	amd64/semantic/misc.cpp
	amd64/semantic/comparison.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC includes)

target_link_libraries(${PROJECT_NAME} VTIL)