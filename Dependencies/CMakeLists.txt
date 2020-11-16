include(FetchContent)

message(STATUS "Fetching VTIL-Core (this might take a while)...")
FetchContent_Declare(
    VTIL-Core
    GIT_REPOSITORY https://github.com/vtil-project/VTIL-Core
    GIT_TAG        eeffb26b8b9ef7b4bd6ffbfc09f3133b70b36eda
    GIT_SHALLOW    false
)
FetchContent_MakeAvailable(VTIL-Core)