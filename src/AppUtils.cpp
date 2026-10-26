/**
 * @file AppUtils.cpp
 *
 * @brief Implementations of the helpers shared by the three programs
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "AppUtils.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

bool openVideoSource(const std::string &source, cv::VideoCapture &capture)
{
    bool is_index = !source.empty() && std::all_of(source.begin(), source.end(), [](char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) != 0;
    });

    if (source.empty() || is_index)
    {
        int index = source.empty() ? 0 : std::stoi(source);
        return capture.open(index);
    }
    return capture.open(source);
}

void chdirToExecutableDir()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
    {
        return;
    }
    wchar_t *slash = wcsrchr(path, L'\\');
    if (slash)
    {
        *slash = L'\0';
        SetCurrentDirectoryW(path);
    }
#else
    char path[4096];
#ifdef __APPLE__
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0)
    {
        return;
    }
#else
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len <= 0)
    {
        return;
    }
    path[len] = '\0';
#endif
    char *slash = std::strrchr(path, '/');
    if (slash)
    {
        *slash = '\0';
        if (chdir(path) != 0)
        {
            return;
        }
    }
#endif
}

bool ensureDataDir()
{
    std::error_code error;
    std::filesystem::create_directories(DATA_DIR, error);
    return std::filesystem::is_directory(DATA_DIR, error);
}

std::string saveImage(const std::string &prefix, const cv::Mat &image)
{
    if (!ensureDataDir())
    {
        return "";
    }

    for (int n = 1; n < 10000; n++)
    {
        char name[256];
        std::snprintf(name, sizeof(name), "%s/%s_%03d.png", DATA_DIR, prefix.c_str(), n);
        if (!std::filesystem::exists(name))
        {
            return cv::imwrite(name, image) ? std::string(name) : std::string();
        }
    }
    return "";
}

void drawHelpText(cv::Mat &image, const std::string &text)
{
    const int font = cv::FONT_HERSHEY_SIMPLEX;
    const double scale = 0.5;
    const int thickness = 1;

    int baseline = 0;
    cv::Size text_size = cv::getTextSize(text, font, scale, thickness, &baseline);
    int strip_height = text_size.height + baseline + 12;

    cv::Rect strip(0, std::max(0, image.rows - strip_height), image.cols,
                   std::min(strip_height, image.rows));
    if (image.channels() == 4)
    {
        // transparent overlay with premultiplied alpha: 65% opaque black
        image(strip).setTo(cv::Scalar(0, 0, 0, 166));
    }
    else
    {
        image(strip) *= 0.35;
    }
    cv::putText(image, text, cv::Point(8, image.rows - baseline - 6), font, scale,
                cv::Scalar(255, 255, 255, 255), thickness, cv::LINE_AA);
}
