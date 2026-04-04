include(FetchContent)

message(STATUS "Fetching VTIL-Core (this might take a while)...")
FetchContent_Declare(
    VTIL-Core
    GIT_REPOSITORY https://github.com/vtil-project/VTIL-Core
    GIT_TAG        0c2e3e122e924510433e344b2f2694ae828d9cab
    GIT_SHALLOW    false
)
FetchContent_MakeAvailable(VTIL-Core)
