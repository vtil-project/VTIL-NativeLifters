include(FetchContent)

message(STATUS "Fetching VTIL-Core (this might take a while)...")
FetchContent_Declare(
    VTIL-Core
    GIT_REPOSITORY https://github.com/vtil-project/VTIL-Core
    GIT_TAG        81144e520a3bfa4e47896f565c98b54a780b2f3d
    GIT_SHALLOW    false
)
FetchContent_MakeAvailable(VTIL-Core)
