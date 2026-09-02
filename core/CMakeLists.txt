cmake_minimum_required(VERSION 3.22.1)

project(ext LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(COMPILE_WARNING_AS_ERROR ON)
set(CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../core/main)

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

file(GLOB_RECURSE CoreSrc ${CORE_SRC}/main/**/*.c)
list(APPEND CoreSrc 
    ${CMAKE_CURRENT_SOURCE_DIR}/c/assets.c
)

add_library(${PROJECT_NAME} SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/c/graphics/graphics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/c/main.c
    ${CoreSrc}
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/c ${CORE_SRC})
target_link_libraries(${PROJECT_NAME} android log m)

add_library(${PROJECT_NAME}_gles SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/c/graphics/graphics_gles.c
    ${CoreSrc}
)
target_include_directories(${PROJECT_NAME}_gles PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/c ${CORE_SRC})
target_link_libraries(${PROJECT_NAME}_gles EGL GLESv3 android log m)


