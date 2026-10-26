/**
 * @file AugmentedReality.cpp
 *
 * @brief Augmented reality (Tasks 4-6 and the OpenGL scene graph extension)
 *
 * Task 4: estimate the board pose (camera position) with solvePnP every frame.
 * Task 5: project 3D axes attached to the board origin.
 * Task 6: draw a virtual wireframe object floating above the board.
 * Extension: render a scene graph model (the Hogwarts castle) standing on the board with
 * OpenGL, using a projection built from the camera calibration.
 *
 * OpenCV does the vision work and draws the Task 5 and 6 lines with projectPoints onto a
 * transparent overlay; ARView composes the undistorted camera image, the OpenGL scene and the
 * overlay in an OpenGL window.
 *
 * Usage: AugmentedReality [camera-index | video-file]
 *
 * @author Yu Cao
 * @date 2026-03-16
 */
#include <cstdio>
#include <exception>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include "ARView.h"
#include "AppUtils.h"
#include "Calibration.h"
#include "gl/CameraMatrices.h"

namespace
{
    const char *WINDOW_NAME = "Augmented Reality";
    const char *SCENE_FILE = "scenegraphmodels/hogwarts.txt";

    // OpenGL clipping planes, in checkerboard squares
    const float NEAR_PLANE = 0.5f;
    const float FAR_PLANE = 1000.0f;

    // overlay colors (BGRA); the alpha channel makes them opaque on the transparent overlay
    const cv::Scalar RED(0, 0, 255, 255);
    const cv::Scalar GREEN(0, 255, 0, 255);
    const cv::Scalar BLUE(255, 0, 0, 255);

    /**
     * @brief Transform from the scene's coordinates to board coordinates: stand the model up
     *        (its Y axis becomes the board's Z axis), move it onto the board, and shrink it so
     *        that 10 scene units are one checkerboard square
     */
    glm::mat4 scenePlacement()
    {
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -60.0f, 0.0f));
        glm::mat4 rotate =
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        return scale * translate * rotate;
    }

    /**
     * @brief Task 4: show the board rotation and translation
     */
    void drawPose(cv::Mat &overlay, const cv::Mat &rvec, const cv::Mat &tvec)
    {
        char rvec_text[128], tvec_text[128];
        std::snprintf(rvec_text, sizeof(rvec_text), "rvec: (%.3f, %.3f, %.3f)", rvec.at<double>(0),
                      rvec.at<double>(1), rvec.at<double>(2));
        std::snprintf(tvec_text, sizeof(tvec_text), "tvec: (%.3f, %.3f, %.3f)", tvec.at<double>(0),
                      tvec.at<double>(1), tvec.at<double>(2));
        cv::putText(overlay, rvec_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, RED, 2,
                    cv::LINE_AA);
        cv::putText(overlay, tvec_text, cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 0.7, RED, 2,
                    cv::LINE_AA);
    }

    /**
     * @brief Task 5: project 2-square-long X (red), Y (green) and Z (blue) axes from the board
     *        origin
     */
    void drawAxes(cv::Mat &overlay, const cv::Mat &rvec, const cv::Mat &tvec,
                  const cv::Mat &camera_matrix, const cv::Mat &dist_coeffs)
    {
        const std::vector<cv::Point3f> axis_points = {{0, 0, 0}, {2, 0, 0}, {0, 2, 0}, {0, 0, 2}};
        std::vector<cv::Point2f> image_points;
        cv::projectPoints(axis_points, rvec, tvec, camera_matrix, dist_coeffs, image_points);

        cv::line(overlay, image_points[0], image_points[1], RED, 3, cv::LINE_AA);
        cv::line(overlay, image_points[0], image_points[2], GREEN, 3, cv::LINE_AA);
        cv::line(overlay, image_points[0], image_points[3], BLUE, 3, cv::LINE_AA);
    }

    /**
     * @brief Task 6: draw a wireframe cube floating one square above the board, with a blue
     *        bottom face, a green top face and red pillars
     */
    void drawVirtualObject(cv::Mat &overlay, const cv::Mat &rvec, const cv::Mat &tvec,
                           const cv::Mat &camera_matrix, const cv::Mat &dist_coeffs)
    {
        // 8 vertices of a 2x2x2 cube; Z > 0 is above the board
        const std::vector<cv::Point3f> cube_points = {
            {2, -2, 1}, {4, -2, 1}, {4, -4, 1}, {2, -4, 1}, // bottom face
            {2, -2, 3}, {4, -2, 3}, {4, -4, 3}, {2, -4, 3}, // top face
        };

        std::vector<cv::Point2f> image_points;
        cv::projectPoints(cube_points, rvec, tvec, camera_matrix, dist_coeffs, image_points);

        for (int i = 0; i < 4; i++)
        {
            cv::line(overlay, image_points[i], image_points[(i + 1) % 4], BLUE, 2, cv::LINE_AA);
            cv::line(overlay, image_points[4 + i], image_points[4 + (i + 1) % 4], GREEN, 2,
                     cv::LINE_AA);
            cv::line(overlay, image_points[i], image_points[4 + i], RED, 2, cv::LINE_AA);
        }
    }
} // namespace

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::printf("Usage: AugmentedReality [camera-index | video-file]  (default: camera 0)\n");
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

    // read the calibration written by Calibrate
    cv::Mat camera_matrix, dist_coeffs;
    cv::Size calibration_size;
    if (!loadIntrinsics(INTRINSICS_FILE, camera_matrix, dist_coeffs, calibration_size))
    {
        std::printf("Error: cannot read %s. Run Calibrate first.\n", INTRINSICS_FILE);
        return 1;
    }

    cv::Mat frame;
    capture >> frame;
    if (frame.empty())
    {
        std::printf("Error: the video source returned no frames\n");
        return 1;
    }
    if (calibration_size.area() > 0 && calibration_size != frame.size())
    {
        std::printf("Warning: calibrated at %dx%d but the video is %dx%d; the pose will be off\n",
                    calibration_size.width, calibration_size.height, frame.cols, frame.rows);
    }

    ARView view;
    try
    {
        view.open(frame.size(), WINDOW_NAME);
    }
    catch (const std::exception &e)
    {
        std::printf("Error: %s\n", e.what());
        return 1;
    }

    // Extension: load the scene graph and upload its meshes once
    try
    {
        view.loadScene(SCENE_FILE, scenePlacement());
        std::printf("Loaded %s\n", SCENE_FILE);
    }
    catch (const std::exception &e)
    {
        std::printf("Warning: scene disabled: %s\n", e.what());
    }

    // The camera image is undistorted before it is shown, so that OpenGL's pinhole projection
    // and projectPoints without distortion both line up with it.
    cv::Mat undistort_map1, undistort_map2;
    cv::initUndistortRectifyMap(camera_matrix, dist_coeffs, cv::Mat(), camera_matrix, frame.size(),
                                CV_16SC2, undistort_map1, undistort_map2);
    const cv::Mat no_distortion = cv::Mat::zeros(5, 1, CV_64F);
    const glm::mat4 projection =
        gl::projectionFromIntrinsics(camera_matrix, frame.size(), NEAR_PLANE, FAR_PLANE);

    bool show_axes = true;
    bool show_object = false;
    bool show_scene = view.isSceneLoaded();
    bool take_screenshot = false;

    std::printf("Controls:\n");
    std::printf("  '1' - toggle 3D axes\n");
    std::printf("  '2' - toggle the virtual wireframe object\n");
    std::printf("  '3' - toggle the scene graph model\n");
    std::printf("  's' - save a screenshot\n");
    std::printf("  'q' - quit\n\n");

    const std::vector<cv::Vec3f> point_set = buildPointSet();
    std::vector<cv::Point2f> corner_set;
    cv::Mat gray, undistorted, rvec, tvec;
    while (!frame.empty() && !view.shouldClose())
    {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        bool found = detectCorners(gray, corner_set);
        if (found)
        {
            // Task 4: board pose relative to the camera, from the raw (distorted) corners
            cv::solvePnP(point_set, corner_set, camera_matrix, dist_coeffs, rvec, tvec);
        }

        cv::remap(frame, undistorted, undistort_map1, undistort_map2, cv::INTER_LINEAR);

        // everything OpenCV draws goes on a transparent overlay above the 3D scene
        cv::Mat overlay(frame.size(), CV_8UC4, cv::Scalar::all(0));
        if (found)
        {
            if (show_object)
            {
                drawVirtualObject(overlay, rvec, tvec, camera_matrix, no_distortion);
            }
            if (show_axes)
            {
                drawAxes(overlay, rvec, tvec, camera_matrix, no_distortion);
            }
            drawPose(overlay, rvec, tvec);
        }
        else
        {
            cv::putText(overlay, "No target", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, RED,
                        2, cv::LINE_AA);
        }
        if (!take_screenshot)
        {
            drawHelpText(overlay, "[1] axes   [2] object   [3] scene   [s] screenshot   [q] quit");
        }

        glm::mat4 board_view;
        if (found && show_scene)
        {
            board_view = gl::viewFromPose(rvec, tvec);
        }
        cv::Mat screenshot;
        view.render(undistorted, overlay, projection, (found && show_scene) ? &board_view : nullptr,
                    take_screenshot ? &screenshot : nullptr);

        if (take_screenshot)
        {
            take_screenshot = false;
            std::string path = saveImage("ar", screenshot);
            if (!path.empty())
            {
                std::printf("Screenshot saved to %s\n", path.c_str());
            }
        }

        for (int key = view.nextKey(); key != -1; key = view.nextKey())
        {
            if (key == 'q' || key == 27)
            {
                return 0;
            }
            else if (key == '1')
            {
                show_axes = !show_axes;
            }
            else if (key == '2')
            {
                show_object = !show_object;
            }
            else if (key == '3')
            {
                show_scene = !show_scene && view.isSceneLoaded();
            }
            else if (key == 's')
            {
                // the next frame is drawn without the help bar and saved
                take_screenshot = true;
            }
        }

        capture >> frame;
    }

    return 0;
}
