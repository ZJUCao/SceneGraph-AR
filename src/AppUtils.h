/**
 * @file AppUtils.h
 *
 * @brief Helpers shared by the three programs: video input, the working directory, the data
 *        folder for outputs, and on-screen help text
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef APPUTILS_H
#define APPUTILS_H

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <string>

// folder next to the executables for calibration results, saved views and screenshots
constexpr const char *DATA_DIR = "data";

/**
 * @brief Open a camera or a video file
 *
 * @param[in]  source   A camera index such as "0" or "1", or the path of a video file;
 *                      an empty string opens camera 0
 * @param[out] capture  The opened capture
 * @return true if the source could be opened
 */
bool openVideoSource(const std::string &source, cv::VideoCapture &capture);

/**
 * @brief Change the working directory to the folder of the executable
 *
 * Models, scene files and the data folder are used with paths relative to the working
 * directory, and the build copies the assets next to the executables. Switching there lets
 * the programs be launched from anywhere.
 */
void chdirToExecutableDir();

/**
 * @brief Create DATA_DIR in the working directory if it does not exist yet
 *
 * @return true if the folder exists afterwards
 */
bool ensureDataDir();

/**
 * @brief Write an image to DATA_DIR as <prefix>_<n>.png, using the first unused n
 *
 * @param[in] prefix  File name prefix
 * @param[in] image   The image to write
 * @return the path written, or an empty string on failure
 */
std::string saveImage(const std::string &prefix, const cv::Mat &image);

/**
 * @brief Draw a line of help text on a dark strip along the bottom of an image
 *
 * @param[in,out] image  The image to draw on: BGR, or BGRA with premultiplied alpha (an
 *                       overlay), where the strip is made translucent instead of darkened
 * @param[in]     text   The text
 */
void drawHelpText(cv::Mat &image, const std::string &text);

#endif
