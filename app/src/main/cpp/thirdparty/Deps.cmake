include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
FetchContent_Declare(
        stb
        GIT_REPOSITORY    https://github.com/nothings/stb.git
        GIT_TAG           b42009b3b9d4ca35bc703f5310eedc74f584be58
        PREFIX            "${CMAKE_SOURCE_DIR}/thirdparty"
        SOURCE_DIR        "${CMAKE_SOURCE_DIR}/thirdparty/stb"
)

FetchContent_Declare(
        glfm
        GIT_REPOSITORY    https://github.com/brackeen/glfm.git
        GIT_TAG           607fc6ff8c89d58641c79adfb00ac9ced3652adc
        PREFIX            "${CMAKE_SOURCE_DIR}/thirdparty"
        SOURCE_DIR        "${CMAKE_SOURCE_DIR}/thirdparty/glfm"
)
FetchContent_MakeAvailable(stb glfm)