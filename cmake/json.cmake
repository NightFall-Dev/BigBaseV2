include(FetchContent)

set(JSON_MultipleHeaders OFF)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d
    GIT_PROGRESS TRUE
)
message("json")
FetchContent_MakeAvailable(json)
