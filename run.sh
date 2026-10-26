#!/usr/bin/env bash
# Build (if needed) and run SceneGraph AR on macOS / Linux.
# Usage: ./run.sh [program] [source]
#   program  calibrate, ar or features (default: ar once a calibration exists, otherwise calibrate)
#   source   camera index (default 0) or a video file
set -e

program="${1:-}"
source="${2:-}"
# Resolve a video file argument before changing directory; camera indices are kept as is
if [ -n "$source" ] && [ -e "$source" ]; then
    source="$(cd "$(dirname "$source")" && pwd)/$(basename "$source")"
fi
cd "$(dirname "$0")"

# CMake from Homebrew or the CMake.app bundle may not be on PATH
for dir in /opt/homebrew/bin /usr/local/bin /Applications/CMake.app/Contents/bin; do
    if ! command -v cmake >/dev/null 2>&1 && [ -x "$dir/cmake" ]; then
        PATH="$dir:$PATH"
    fi
done

if [ "$(uname -s)" = "Darwin" ] && ! xcode-select -p >/dev/null 2>&1; then
    echo "[run] Xcode Command Line Tools not found. Install them with: xcode-select --install" >&2
    exit 1
fi
for tool in cmake git make c++; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[run] $tool not found. See \"Requirements\" in README.md." >&2
        exit 1
    fi
done

if [ -z "$program" ]; then
    if [ -f build/bin/data/intrinsics.yml ]; then program=ar; else program=calibrate; fi
fi
case "$program" in
    calibrate) exe=Calibrate ;;
    ar)        exe=AugmentedReality ;;
    features)  exe=HarrisFeatures ;;
    *)
        echo "[run] Unknown program \"$program\". Usage: ./run.sh [calibrate|ar|features] [camera-index|video-file]" >&2
        exit 1
        ;;
esac

if [ ! -f build/Makefile ]; then
    if ! cmake --preset default; then
        echo "[run] Configure failed. OpenCV must be installed, and on Linux GLFW needs the X11 and" >&2
        echo "[run] Wayland development packages; see \"Requirements\" in README.md." >&2
        exit 1
    fi
fi
cmake --build --preset default -j "$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

case "$exe" in
    Calibrate)
        echo "[run] Show the 9x6 checkerboard from different angles: S saves a view, C calibrates after 5+ views, Q quits." ;;
    AugmentedReality)
        echo "[run] Point the camera at the checkerboard: 1/2/3 toggle axes/object/scene, S saves a screenshot, Q quits." ;;
esac
if [ -n "$source" ]; then
    exec "./build/bin/$exe" "$source"
else
    exec "./build/bin/$exe"
fi
