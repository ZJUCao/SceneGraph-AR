/**
 * @file CameraMatrices.cpp
 *
 * @brief OpenGL projection and view matrices from an OpenCV camera calibration and pose
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "gl/CameraMatrices.h"

#include <opencv2/calib3d.hpp>

namespace gl
{
    glm::mat4 projectionFromIntrinsics(const cv::Mat &camera_matrix, cv::Size image_size,
                                       float near_plane, float far_plane)
    {
        cv::Mat k;
        camera_matrix.convertTo(k, CV_64F);
        const float fx = static_cast<float>(k.at<double>(0, 0));
        const float fy = static_cast<float>(k.at<double>(1, 1));
        const float cx = static_cast<float>(k.at<double>(0, 2));
        const float cy = static_cast<float>(k.at<double>(1, 2));
        const float w = static_cast<float>(image_size.width);
        const float h = static_cast<float>(image_size.height);
        const float n = near_plane;
        const float f = far_plane;

        // Pixel u covers [u - 0.5, u + 0.5], so the image spans x in [-0.5, w - 0.5]; the
        // same holds for v, and image row 0 is at the top (NDC y = +1).
        glm::mat4 p(0.0f); // glm is column-major: p[column][row]
        p[0][0] = 2.0f * fx / w;
        p[1][1] = 2.0f * fy / h;
        p[2][0] = 1.0f - 2.0f * (cx + 0.5f) / w;
        p[2][1] = 2.0f * (cy + 0.5f) / h - 1.0f;
        p[2][2] = -(f + n) / (f - n);
        p[2][3] = -1.0f;
        p[3][2] = -2.0f * f * n / (f - n);
        return p;
    }

    glm::mat4 viewFromPose(const cv::Mat &rvec, const cv::Mat &tvec)
    {
        cv::Mat rotation, translation;
        cv::Rodrigues(rvec, rotation);
        rotation.convertTo(rotation, CV_64F);
        tvec.convertTo(translation, CV_64F);

        // [R | t] in OpenCV camera coordinates, then flip Y and Z for OpenGL
        const float flip[3] = {1.0f, -1.0f, -1.0f};
        glm::mat4 view(1.0f);
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                view[col][row] = flip[row] * static_cast<float>(rotation.at<double>(row, col));
            }
            view[3][row] = flip[row] * static_cast<float>(translation.at<double>(row));
        }
        return view;
    }
} // namespace gl
