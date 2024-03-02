
if(NOT "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/src/glfm-populate-stamp/glfm-populate-gitinfo.txt" IS_NEWER_THAN "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/src/glfm-populate-stamp/glfm-populate-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/src/glfm-populate-stamp/glfm-populate-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/glfm"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/glfm'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"  clone --no-checkout --config "advice.detachedHead=false" "https://github.com/brackeen/glfm.git" "glfm"
    WORKING_DIRECTORY "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/brackeen/glfm.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  checkout 607fc6ff8c89d58641c79adfb00ac9ced3652adc --
  WORKING_DIRECTORY "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/glfm"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: '607fc6ff8c89d58641c79adfb00ac9ced3652adc'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git"  submodule update --recursive --init 
    WORKING_DIRECTORY "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/glfm"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/glfm'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/src/glfm-populate-stamp/glfm-populate-gitinfo.txt"
    "/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/src/glfm-populate-stamp/glfm-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/Users/renruisi/Downloads/glfw-android-example/app/src/main/cpp/thirdparty/src/glfm-populate-stamp/glfm-populate-gitclone-lastrun.txt'")
endif()

