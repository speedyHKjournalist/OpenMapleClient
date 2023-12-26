include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
FetchContent_Declare(
        stb
        GIT_REPOSITORY    https://github.com/nothings/stb.git
        GIT_TAG           b42009b3b9d4ca35bc703f5310eedc74f584be58
        PREFIX            "${CMAKE_SOURCE_DIR}/thirdparty"
        SOURCE_DIR        "${CMAKE_SOURCE_DIR}/thirdparty/stb"
)
FetchContent_MakeAvailable(stb)