# Install script for directory: /Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/freetype

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/renruisi/Library/Android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xheadersx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype2" TYPE DIRECTORY FILES "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/freetype/include/" REGEX "/internal$" EXCLUDE REGEX "/ftconfig\\.h$" EXCLUDE REGEX "/ftoption\\.h$" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xheadersx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype2/freetype/config" TYPE FILE FILES
    "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/include/freetype/config/ftconfig.h"
    "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/include/freetype/config/ftoption.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xpkgconfigx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/freetype2.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/libfreetyped.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xheadersx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype/freetype-config.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype/freetype-config.cmake"
         "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/CMakeFiles/Export/lib/cmake/freetype/freetype-config.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype/freetype-config-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype/freetype-config.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype" TYPE FILE FILES "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/CMakeFiles/Export/lib/cmake/freetype/freetype-config.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype" TYPE FILE FILES "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/CMakeFiles/Export/lib/cmake/freetype/freetype-config-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xheadersx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/freetype" TYPE FILE FILES "/Users/renruisi/Downloads/glfw-android-example/app/.cxx/Debug/295s6r6h/arm64-v8a/thirdparty/freetype/freetype-config-version.cmake")
endif()

