aux_source_directory(. NOLIFENX_SOURCES)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)
add_library(NoLifeNx STATIC
        ${NOLIFENX_SOURCES}
        ${CMAKE_SOURCE_DIR}/thirdparty/nlnx/lz4/lib/lz4.c)
target_include_directories(NoLifeNx
        PRIVATE
        .
        ${CMAKE_SOURCE_DIR}/thirdparty/nlnx/lz4/lib
        )