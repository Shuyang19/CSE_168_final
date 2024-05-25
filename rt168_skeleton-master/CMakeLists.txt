cmake_minimum_required(VERSION 2.8)
project(rt168 CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    set(ARCH_LIBRARIES "")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=unused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(ARCH_LIBRARIES "pthread")
endif()

find_package(Embree REQUIRED)

set(LIB_SOURCES
    src/Engine.cpp
    src/Scene.cpp
    src/SceneLoader.cpp
    src/RenderPool.cpp
    src/RayTracerIntegrator.cpp
    src/AnalyticdirectIntegrator.cpp
    src/Pathtracer.cpp
    src/directIntegrator.cpp
    src/lodepng/lodepng.cpp
)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
include_directories(src/ ${EMBREE_INCLUDE_PATH})
add_library(rt168_lib ${LIB_SOURCES})
set_target_properties(rt168_lib PROPERTIES OUTPUT_NAME rt168)
target_link_libraries(rt168_lib ${ARCH_LIBRARIES} ${EMBREE_LIBRARY})

set(CLI_SOURCES
    src/CliMain.cpp
)
add_executable(rt168_cli ${CLI_SOURCES})
set_target_properties(rt168_cli PROPERTIES OUTPUT_NAME rt168)
target_link_libraries(rt168_cli rt168_lib)

if (WIN32)
    set(WIN_SOURCES
        src/WindowsMain.cpp
    )
    add_executable(rt168_win ${WIN_SOURCES})
    target_link_libraries(rt168_win rt168_lib)
endif()

if (WIN32)
    add_custom_command(TARGET rt168_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/embree/bin"
        "$<TARGET_FILE_DIR:rt168_lib>")
endif()