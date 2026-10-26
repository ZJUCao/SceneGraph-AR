/**
 * @file Calibrate.cpp
 *
 * @brief Camera calibration (Tasks 1-3)
 *
 * Task 1: detect and draw the checkerboard corners in live video.
 * Task 2: let the user select calibration views (press 's').
 * Task 3: run the calibration (press 'c') and write the intrinsics to data/intrinsics.yml.
 *
 * Usage: Calibrate [camera-index | video-file]
 *
 * @author Na Yin, Yu Cao
 * @date 2026-03-12
 */
#include <cstdio>
#include <string>
#include <vector>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "AppUtils.h"
#include "Calibration.h"

namespace
{
    const char *WINDOW_NAME = "Calibrate";

    /**
     * @brief Print the 2D <-> 3D correspondences of one saved view
     */
    void printCorrespondences(int view_number, const std::vector<cv::Point2f> &corner_set,
                              const std::vector<cv::Vec3f> &point_set)
    {
        std::printf("\n=== View %d saved ===\n", view_number);
        std::printf("  [idx]  (u, v) <-> (X, Y, Z)\n");
        for (size_t i = 0; i < corner_set.size(); i++)
        {
            std::printf("  [%2d]  (%.2f, %.2f) <-> (%.1f, %.1f, %.1f)\n", static_cast<int>(i),
                        corner_set[i].x, corner_set[i].y, point_set[i][0], point_set[i][1],
                        point_set[i][2]);
        }
        std::printf("\n");
    }
} // namespace

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::printf("Usage: Calibrate [camera-index | video-file]  (default: camera 0)\n");
        return 1;
    }

    // open the source before changing directory, so relative video paths still work
    cv::VideoCapture capture;
    if (!openVideoSource(argc == 2 ? argv[1] : "", capture))
    {
        std::printf("Error: cannot open video source %s\n", argc == 2 ? argv[1] : "0");
        return 1;
    }
    chdirToExecutableDir();

    // accumulation lists for calibration (Tasks 2 & 3)
    std::vector<std::vector<cv::Point2f>> corner_list;
    std::vector<std::vector<cv::Vec3f>> point_list;

    // most recent detection, saved on 's'
    std::vector<cv::Point2f> corner_set;
    cv::Mat camera_matrix, dist_coeffs;

    std::printf("Show the %dx%d checkerboard to the camera from different angles.\n", BOARD_COLS,
                BOARD_ROWS);
    std::printf("Controls:\n");
    std::printf("  's' - save the current view for calibration\n");
    std::printf("  'c' - run calibration (needs >= %d views) and write %s\n", MIN_CALIB_FRAMES,
                INTRINSICS_FILE);
    std::printf("  'q' - quit\n\n");

    cv::Mat frame, gray;
    while (true)
    {
        capture >> frame;
        if (frame.empty())
        {
            break;
        }

        // Task 1: detect corners every frame and draw them when found
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        bool found = detectCorners(gray, corner_set);
        if (found)
        {
            cv::drawChessboardCorners(frame, cv::Size(BOARD_COLS, BOARD_ROWS), corner_set, found);
        }

        // the corner count and the first corner show whether the ordering is stable
        cv::Mat display = frame.clone();
        if (found)
        {
            char text[128];
            std::snprintf(text, sizeof(text), "Corners: %d   first: (%.1f, %.1f)",
                          static_cast<int>(corner_set.size()), corner_set[0].x, corner_set[0].y);
            cv::putText(display, text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        cv::Scalar(0, 255, 0), 2);
        }
        drawHelpText(display, "Views: " + std::to_string(corner_list.size()) + "/" +
                                  std::to_string(MIN_CALIB_FRAMES) +
                                  "   [s] save view   [c] calibrate   [q] quit");
        cv::imshow(WINDOW_NAME, display);

        int key = cv::waitKey(10);
        if (key == 'q' || key == 27)
        {
            break;
        }
        else if (key == 's')
        {
            // Task 2: keep the corners of the last successful detection
            if (!found)
            {
                std::printf("No board detected - view not saved\n");
                continue;
            }
            addCalibrationFrame(corner_set, corner_list, point_list);
            printCorrespondences(static_cast<int>(corner_list.size()), corner_list.back(),
                                 point_list.back());

            std::string image_path = saveImage("calib", frame);
            if (!image_path.empty())
            {
                std::printf("View image saved to %s\n", image_path.c_str());
            }
        }
        else if (key == 'c')
        {
            // Task 3: calibrate with all saved views, starting from K with the principal
            // point at the image center
            if (camera_matrix.empty())
            {
                // clang-format off
                camera_matrix = (cv::Mat_<double>(3, 3) << 1, 0, frame.cols / 2.0,
                                                           0, 1, frame.rows / 2.0,
                                                           0, 0, 1);
                // clang-format on
            }

            double error =
                runCalibration(corner_list, point_list, frame.size(), camera_matrix, dist_coeffs);
            if (error < 0)
            {
                std::printf("Need at least %d views (have %d)\n", MIN_CALIB_FRAMES,
                            static_cast<int>(corner_list.size()));
            }
            else if (ensureDataDir() && saveIntrinsics(INTRINSICS_FILE, camera_matrix, dist_coeffs,
                                                       frame.size(), error))
            {
                std::printf("Calibration done (error %.4f px). Intrinsics saved to %s\n", error,
                            INTRINSICS_FILE);
            }
            else
            {
                std::printf("Error: cannot write %s\n", INTRINSICS_FILE);
            }
        }
    }

    cv::destroyAllWindows();
    return 0;
}
