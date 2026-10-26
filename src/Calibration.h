/**
 * @file Calibration.h
 *
 * @brief Checkerboard target definition and camera calibration helpers
 *
 * Shared by all programs: the checkerboard size, its 3D world points, corner detection, the
 * cv::calibrateCamera wrapper, and reading and writing the intrinsic parameters.
 *
 * @author Na Yin, Yu Cao
 * @date 2026-03-12
 */
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <opencv2/core.hpp>
#include <string>
#include <vector>

// checkerboard inner corner dimensions (9 cols x 6 rows)
constexpr int BOARD_COLS = 9;
constexpr int BOARD_ROWS = 6;

// cv::calibrateCamera is only run once at least this many views have been saved
constexpr int MIN_CALIB_FRAMES = 5;

// intrinsic parameters written by Calibrate and read by AugmentedReality
constexpr const char *INTRINSICS_FILE = "data/intrinsics.yml";

/**
 * @brief Build the fixed 3D world point set for one checkerboard view.
 *        Origin at top-left inner corner, one unit = one square side.
 *        X goes right, Y goes up (negative row), Z = 0 (planar target), so Z points out of
 *        the board toward the camera. The set is identical for every view since world
 *        coordinates are attached to the target, not the camera.
 *
 * @return vector of 3D world points in row-major order, left to right
 */
std::vector<cv::Vec3f> buildPointSet();

/**
 * @brief Detect inner corners of a checkerboard in a grayscale frame.
 *        Uses findChessboardCorners for initial detection and cornerSubPix
 *        to refine to subpixel accuracy for better calibration quality.
 *        The first corner is generally in the upper-left of the pattern.
 *
 * @param[in]  gray        Grayscale input frame
 * @param[out] corner_set  Detected and refined 2D corner positions (pixel coords)
 * @return true if all BOARD_COLS x BOARD_ROWS corners were found, false otherwise
 */
bool detectCorners(const cv::Mat &gray, std::vector<cv::Point2f> &corner_set);

/**
 * @brief Add one calibration view: append the detected 2D corners and the corresponding
 *        fixed 3D world points to the accumulation lists. Both lists stay the same length,
 *        one entry per saved view.
 *
 * @param[in]     corner_set   Detected 2D corner positions for this view
 * @param[in,out] corner_list  Accumulation list of 2D corners across all views
 * @param[in,out] point_list   Accumulation list of 3D world points across all views
 */
void addCalibrationFrame(const std::vector<cv::Point2f> &corner_set,
                         std::vector<std::vector<cv::Point2f>> &corner_list,
                         std::vector<std::vector<cv::Vec3f>> &point_list);

/**
 * @brief Run camera calibration using all saved views.
 *        Calls cv::calibrateCamera to solve for the intrinsic matrix K and the distortion
 *        coefficients, and prints the camera matrix before and after along with the
 *        distortion coefficients and the reprojection error.
 *
 * @param[in]     corner_list   All saved 2D corner sets (one per view)
 * @param[in]     point_list    All saved 3D world point sets (one per view)
 * @param[in]     image_size    Resolution of the calibration images
 * @param[in,out] camera_matrix Initial K estimate before the call, optimized K after
 * @param[out]    dist_coeffs   Distortion coefficients
 * @return reprojection error in pixels (< 1.0 is good), or -1 if there are not enough views
 */
double runCalibration(const std::vector<std::vector<cv::Point2f>> &corner_list,
                      const std::vector<std::vector<cv::Vec3f>> &point_list, cv::Size image_size,
                      cv::Mat &camera_matrix, cv::Mat &dist_coeffs);

/**
 * @brief Write the intrinsic parameters to a YAML file
 *
 * @param[in] path               Output file
 * @param[in] camera_matrix      3x3 camera matrix K
 * @param[in] dist_coeffs        Distortion coefficients
 * @param[in] image_size         Resolution the calibration was done at
 * @param[in] reprojection_error Reprojection error of the calibration, in pixels
 * @return true on success
 */
bool saveIntrinsics(const std::string &path, const cv::Mat &camera_matrix,
                    const cv::Mat &dist_coeffs, cv::Size image_size, double reprojection_error);

/**
 * @brief Read the intrinsic parameters written by saveIntrinsics
 *
 * @param[in]  path          Input file
 * @param[out] camera_matrix 3x3 camera matrix K
 * @param[out] dist_coeffs   Distortion coefficients
 * @param[out] image_size    Resolution the calibration was done at
 * @return true if the file exists and contains a 3x3 camera matrix
 */
bool loadIntrinsics(const std::string &path, cv::Mat &camera_matrix, cv::Mat &dist_coeffs,
                    cv::Size &image_size);

#endif
