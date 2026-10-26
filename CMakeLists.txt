cmake_minimum_required(VERSION 3.21)

project(SceneGraphAR LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Corner detection and the Harris pipeline are noticeably slower without
# optimization. Default to Release unless a build type was chosen explicitly.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(cmake/Dependencies.cmake)

# All executables, the copied assets and (on Windows) the OpenCV DLLs go to
# build/bin. The generator expression keeps multi-config generators (Visual
# Studio) from adding a Release/ or Debug/ subfolder.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin$<0:>")
set(SGAR_OUTPUT_DIR "${PROJECT_BINARY_DIR}/bin")

# Code shared by the three programs: calibration helpers and video / file
# utilities (OpenCV only).
add_library(sgar_common STATIC
    src/AppUtils.cpp
    src/Calibration.cpp
)
target_include_directories(sgar_common PUBLIC src)
target_link_libraries(sgar_common PUBLIC ${OpenCV_LIBS})

# The OpenGL side of the AR program: window, scene graph renderer and helpers.
add_library(sgar_render STATIC
    src/ARView.cpp
    src/SceneRenderer.cpp
    src/gl/CameraMatrices.cpp
    src/gl/Mesh.cpp
    src/gl/ShaderProgram.cpp
    src/gl/Texture.cpp
)
target_link_libraries(sgar_render PUBLIC sgar_common glad glfw glm::glm)

add_executable(Calibrate        src/apps/Calibrate.cpp)
add_executable(AugmentedReality src/apps/AugmentedReality.cpp)
add_executable(HarrisFeatures   src/apps/HarrisFeatures.cpp)
set(SGAR_PROGRAMS Calibrate AugmentedReality HarrisFeatures)

target_link_libraries(Calibrate        PRIVATE sgar_common)
target_link_libraries(AugmentedReality PRIVATE sgar_render)
target_link_libraries(HarrisFeatures   PRIVATE sgar_common)

# The official Windows OpenCV package decodes most video formats with an FFmpeg
# plugin DLL that sits next to opencv_world; it is copied along with the DLLs
# below so video files can be opened without OpenCV's bin folder on PATH.
set(SGAR_OPENCV_PLUGINS "")
if(WIN32 AND TARGET opencv_world)
    get_target_property(_opencv_world_dll opencv_world IMPORTED_LOCATION_RELEASE)
    if(_opencv_world_dll)
        get_filename_component(_opencv_bin "${_opencv_world_dll}" DIRECTORY)
        file(GLOB SGAR_OPENCV_PLUGINS "${_opencv_bin}/opencv_videoio_ffmpeg*.dll")
    endif()
endif()

foreach(target sgar_common sgar_render ${SGAR_PROGRAMS})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endforeach()

foreach(program ${SGAR_PROGRAMS})
    set_target_properties(${program} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${SGAR_OUTPUT_DIR}"
    )
    # Windows: copy the OpenCV DLLs next to the executable so it starts without
    # OpenCV's bin folder on PATH
    if(WIN32)
        add_custom_command(TARGET ${program} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_RUNTIME_DLLS:${program}> ${SGAR_OPENCV_PLUGINS}
                    $<TARGET_FILE_DIR:${program}>
            COMMAND_EXPAND_LISTS
        )
    endif()
endforeach()

# The AR program loads shaders/, models/ and scenegraphmodels/ using paths
# relative to the working directory, so copy assets next to the binaries. A
# separate target runs on every build, so editing only an asset (e.g. a shader)
# still updates the copy even when the executables are up to date.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.26)
    set(_copy_cmd copy_directory_if_different)
else()
    set(_copy_cmd copy_directory)
endif()
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E ${_copy_cmd} "${PROJECT_SOURCE_DIR}/assets" "${SGAR_OUTPUT_DIR}"
    COMMENT "Copying assets next to the executables"
)
add_dependencies(AugmentedReality copy_assets)
