project(NativeLifters-Core)

add_library(${PROJECT_NAME} STATIC
	amd64/amd64.cpp
	amd64/semantic/arithmetic.cpp
	amd64/semantic/branch.cpp
	amd64/semantic/comparison.cpp
	amd64/semantic/flags.cpp
	amd64/semantic/misc.cpp
	core/recursive_descent.cpp
	)

target_include_directories(${PROJECT_NAME} PUBLIC includes)

target_link_libraries(${PROJECT_NAME} VTIL)