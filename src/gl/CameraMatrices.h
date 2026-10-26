/**
 * @file CameraMatrices.h
 *
 * @brief OpenGL projection and view matrices from an OpenCV camera calibration and pose
 *
 * OpenCV's camera looks down +Z with Y pointing down the image; OpenGL's looks down -Z with Y
 * up. The view matrix flips Y and Z, and the projection matrix reproduces the pinhole model
 * u = fx * x / z + cx, v = fy * y / z + cy, so that a point rendered by OpenGL lands on the
 * same pixel cv::projectPoints (without distortion) gives it.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef GL_CAMERAMATRICES_H
#define GL_CAMERAMATRICES_H

#include <glm/glm.hpp>
#include <opencv2/core.hpp>

namespace gl
{
    /**
     * @brief Perspective projection matching a pinhole camera
     *
     * @param[in] camera_matrix  3x3 camera matrix K (CV_64F)
     * @param[in] image_size     Image size in pixels
     * @param[in] near_plane     Distance of the near clipping plane, in world units
     * @param[in] far_plane      Distance of the far clipping plane, in world units
     * @return the OpenGL projection matrix
     */
    glm::mat4 projectionFromIntrinsics(const cv::Mat &camera_matrix, cv::Size image_size,
                                       float near_plane, float far_plane);

    /**
     * @brief View (world to OpenGL eye) matrix from a solvePnP pose
     *
     * @param[in] rvec  Rotation as a Rodrigues vector
     * @param[in] tvec  Translation
     * @return the OpenGL view matrix
     */
    glm::mat4 viewFromPose(const cv::Mat &rvec, const cv::Mat &tvec);
} // namespace gl

#endif
