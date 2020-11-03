include(FetchContent)

message(STATUS "Fetching VTIL-Core (this might take a while)...")
FetchContent_Declare(
    VTIL-Core
    GIT_REPOSITORY https://github.com/vtil-project/VTIL-Core
    GIT_TAG        32e5e2d9bc5b88e245e9158092e60d0dd42d6774
    GIT_SHALLOW    false
)
FetchContent_MakeAvailable(VTIL-Core)