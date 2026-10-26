# Third-party dependencies, all declared in one place.
#
#   OpenCV - computer vision library    (installed on the system, found with find_package)
#   glfw   - window/context/input       (fetched from GitHub, built from source)
#   glm    - math library, header-only  (fetched from GitHub)
#   glad   - OpenGL 4.0 core loader     (generated code, vendored in third_party/glad)
#
# To change the GLFW or GLM version, edit its GIT_TAG below and re-run CMake configure.

# --- OpenCV -----------------------------------------------------------------
# Windows: the official package (opencv-4.x-windows.exe) extracts to <dir>\opencv; its
# OpenCVConfig.cmake is in <dir>\opencv\build. Set the OpenCV_DIR environment variable to that
# folder, or add <dir>\opencv\build\x64\vc16\bin to PATH as the OpenCV docs suggest; both are
# picked up here. C:\opencv and C:\tools\opencv (Chocolatey) are also searched.
# macOS / Linux: the Homebrew and distribution packages are found automatically.
set(_opencv_hints "")
if(WIN32)
    list(APPEND _opencv_hints "C:/opencv/build" "C:/tools/opencv/build")
    file(TO_CMAKE_PATH "$ENV{PATH}" _path_dirs)
    foreach(_dir IN LISTS _path_dirs)
        # <dir>\opencv\build\x64\<vc>\bin -> <dir>\opencv\build
        if(_dir MATCHES "^(.*)/x64/vc[0-9]+/bin/?$")
            set(_root "${CMAKE_MATCH_1}")
            if(EXISTS "${_root}/OpenCVConfig.cmake")
                list(APPEND _opencv_hints "${_root}")
            endif()
        endif()
    endforeach()
endif()

set(_opencv_modules core imgproc imgcodecs videoio highgui calib3d)
find_package(OpenCV QUIET COMPONENTS ${_opencv_modules} HINTS ${_opencv_hints})
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR
        "OpenCV was not found. Install it (see \"Requirements\" in README.md); on Windows,\n"
        "set the OpenCV_DIR environment variable to the folder containing OpenCVConfig.cmake,\n"
        "e.g. C:\\opencv\\build, then delete the build folder and configure again.")
endif()
message(STATUS "Found OpenCV ${OpenCV_VERSION}: ${OpenCV_DIR}")

include(FetchContent)

# --- GLFW -------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# --- GLM --------------------------------------------------------------------
# Only the headers are needed, so GLM's own CMake project is not added to the
# build (SOURCE_SUBDIR points at a folder without a CMakeLists.txt); a small
# interface target exposes the headers instead.
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  headers-only
)

FetchContent_MakeAvailable(glfw glm)

add_library(glm INTERFACE)
add_library(glm::glm ALIAS glm)
target_include_directories(glm SYSTEM INTERFACE "${glm_SOURCE_DIR}")

# --- glad -------------------------------------------------------------------
# Regenerate at https://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D4.0
add_library(glad STATIC ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c)
target_include_directories(glad SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/third_party/glad/include)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})
