cmake_minimum_required(VERSION 3.22.1)

project(ext LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(COMPILE_WARNING_AS_ERROR ON)

set(HERE ${CMAKE_CURRENT_SOURCE_DIR}/c)
set(CORE ${CMAKE_CURRENT_SOURCE_DIR}/../core)

add_compile_definitions(
    ANDROID_STL=c_static
    _IDENTITY_="${PROJECT_NAME}"
    $<IF:$<CONFIG:Debug>,DEBUG,>
)
add_compile_options(
    -fexceptions
    -frtti
)
add_link_options(-u ANativeActivity_onCreate)

file(GLOB_RECURSE CoreSrc
    ${CORE}/main/**/*.c
    ${CORE}/ext/**/*.c
)
list(APPEND CoreSrc 
    ${HERE}/assets.c
)

add_library(${PROJECT_NAME} SHARED
    ${HERE}/graphics/graphics.c
    ${HERE}/main.c
    ${CoreSrc}
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${HERE}
    ${CORE}/ext
    ${CORE}/main
)
target_link_libraries(${PROJECT_NAME}
    android
    log
    m
    c
)

add_library(${PROJECT_NAME}_gles SHARED
    ${HERE}/graphics/graphics_gles.c
    ${CoreSrc}
)
target_include_directories(${PROJECT_NAME}_gles PRIVATE
    ${HERE}
    ${CORE}/main
)
target_link_libraries(${PROJECT_NAME}_gles
    EGL
    GLESv3
    android
    log
    m
    c
)


