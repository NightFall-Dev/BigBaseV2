include(FetchContent)

add_compile_definitions(CXX_FORMAT_SUPPORT)

message("AsyncLogger")
set(USE_FMT ON)
FetchContent_Declare(
    AsyncLogger
    GIT_REPOSITORY https://github.com/NightFall-Dev/AsyncLogger.git
    GIT_TAG 8903e093add346982b9c3ce7210ba817dadc9511
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(AsyncLogger)

set_property(TARGET AsyncLogger PROPERTY CXX_STANDARD 23)
