# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.22.1-g37088a8)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(stb-populate NONE)


# Pass through things we've already detected in the main project to avoid
# paying the cost of redetecting them again in ExternalProject_Add()
set(GIT_EXECUTABLE [==[/usr/bin/git]==])
set(GIT_VERSION_STRING [==[2.39.3 (Apple Git-145)]==])
set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  [==[/usr/bin/git;2.39.3 (Apple Git-145)]==]
)


include(ExternalProject)
ExternalProject_Add(stb-populate
                     "UPDATE_DISCONNECTED" "True" "GIT_REPOSITORY" "https://github.com/nothings/stb.git" "GIT_TAG" "b42009b3b9d4ca35bc703f5310eedc74f584be58" "PREFIX" "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty"
                    SOURCE_DIR          "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/stb"
                    BINARY_DIR          "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/_deps/stb-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)


