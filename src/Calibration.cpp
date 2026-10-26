/**
 * @file Calibration.cpp
 *
 * @brief Implementations of the shared camera calibration helpers
 *
 * @author Na Yin, Yu Cao
 * @date 2026-03-12
 */
#include "Calibration.h"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

std::vector<cv::Vec3f> buildPointSet()
{
    std::vector<cv::Vec3f> point_set;

    // assign (col, -row, 0) to each inner corner in row-major order
    // one unit = one checkerboard square side length
    for (int r = 0; r < BOARD_ROWS; r++)
    {
        for (int c = 0; c < BOARD_COLS; c++)
        {
            point_set.push_back(cv::Vec3f(static_cast<float>(c), static_cast<float>(-r), 0.0f));
        }
    }
    return point_set;
}

bool detectCorners(const cv::Mat &gray, std::vector<cv::Point2f> &corner_set)
{
    // adaptive threshold handles uneven lighting, normalize_image handles over/under
    // exposure, fast_check skips frames with no board quickly
    bool found = cv::findChessboardCorners(
        gray, cv::Size(BOARD_COLS, BOARD_ROWS), corner_set,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

    if (found)
    {
        // refine to subpixel accuracy
        // window (11,11): search neighborhood per corner
        // zero zone (-1,-1): no ignored center zone
        // stop after 30 iterations or movement < 0.001 px
        cv::cornerSubPix(
            gray, corner_set, cv::Size(11, 11), cv::Size(-1, -1),
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.001));
    }

    return found;
}

void addCalibrationFrame(const std::vector<cv::Point2f> &corner_set,
                         std::vector<std::vector<cv::Point2f>> &corner_list,
                         std::vector<std::vector<cv::Vec3f>> &point_list)
{
    // the 3D world points are the same for every view
    corner_list.push_back(corner_set);
    point_list.push_back(buildPointSet());
}

double runCalibration(const std::vector<std::vector<cv::Point2f>> &corner_list,
                      const std::vector<std::vector<cv::Vec3f>> &point_list, cv::Size image_size,
                      cv::Mat &camera_matrix, cv::Mat &dist_coeffs)
{
    if (static_cast<int>(corner_list.size()) < MIN_CALIB_FRAMES)
    {
        return -1.0;
    }

    std::vector<cv::Mat> rvecs, tvecs;
    dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);

    std::cout << "\n--- Camera matrix BEFORE calibration ---\n" << camera_matrix << std::endl;
    std::cout << "\n--- Distortion coefficients BEFORE calibration ---\n"
              << dist_coeffs.t() << std::endl;

    double reprojection_error = cv::calibrateCamera(point_list,    // 3D world points per view
                                                    corner_list,   // 2D image points per view
                                                    image_size,    // image resolution
                                                    camera_matrix, // in: initial K, out: K
                                                    dist_coeffs,   // out: distortion
                                                    rvecs,         // out: rotation per view
                                                    tvecs);        // out: translation per view

    std::cout << "\n--- Camera matrix AFTER calibration ---\n" << camera_matrix << std::endl;
    std::cout << "\n--- Distortion coefficients AFTER calibration (k1 k2 p1 p2 k3 k4 k5 k6) ---\n"
              << dist_coeffs.t() << std::endl;
    std::printf("\nReprojection error: %.4f px\n\n", reprojection_error);

    return reprojection_error;
}

bool saveIntrinsics(const std::string &path, const cv::Mat &camera_matrix,
                    const cv::Mat &dist_coeffs, cv::Size image_size, double reprojection_error)
{
    cv::FileStorage fs(path, cv::FileStorage::WRITE);
    if (!fs.isOpened())
    {
        return false;
    }
    fs << "image_width" << image_size.width;
    fs << "image_height" << image_size.height;
    fs << "camera_matrix" << camera_matrix;
    fs << "dist_coeffs" << dist_coeffs;
    fs << "reprojection_error" << reprojection_error;
    return true;
}

bool loadIntrinsics(const std::string &path, cv::Mat &camera_matrix, cv::Mat &dist_coeffs,
                    cv::Size &image_size)
{
    // checked first because cv::FileStorage logs its own error for a missing file
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
        return false;
    }

    cv::FileStorage fs(path, cv::FileStorage::READ);
    if (!fs.isOpened())
    {
        return false;
    }
    fs["camera_matrix"] >> camera_matrix;
    fs["dist_coeffs"] >> dist_coeffs;
    fs["image_width"] >> image_size.width;
    fs["image_height"] >> image_size.height;
    return camera_matrix.rows == 3 && camera_matrix.cols == 3;
}
