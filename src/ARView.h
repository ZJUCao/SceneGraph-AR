/**
 * @file ARView.h
 *
 * @brief The ARView class
 *
 * The OpenGL window of the AR program. Each frame is composed of three layers, drawn into a
 * viewport with the video's aspect ratio:
 *   1. the (undistorted) camera image, as a texture filling the viewport;
 *   2. the scene graph, rendered with depth testing and lighting through a projection that
 *      matches the calibrated camera;
 *   3. an overlay drawn with OpenCV (axes, the wireframe object, text), with premultiplied
 *      alpha, so it stays on top.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef ARVIEW_H
#define ARVIEW_H

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <deque>
#include <glm/glm.hpp>
#include <opencv2/core.hpp>
#include <string>
#include "SceneRenderer.h"
#include "gl/ShaderProgram.h"
#include "gl/Texture.h"

class ARView
{
public:
    ARView() = default;
    ~ARView();

    ARView(const ARView &) = delete;
    ARView &operator=(const ARView &) = delete;

    /**
     * @brief Create the window and the OpenGL 3.3 core context, and load the image shaders
     *
     * The window starts at the video's size, scaled down if it does not fit on the screen.
     *
     * @param[in] frame_size  Size of the video frames
     * @param[in] title       Window title
     * @throws std::runtime_error if the window or the context cannot be created
     */
    void open(cv::Size frame_size, const std::string &title);

    /**
     * @brief Load the scene graph drawn on the board
     *
     * @throws std::runtime_error if the scene or its shaders cannot be loaded
     */
    void loadScene(const std::string &scene_file, const glm::mat4 &placement);

    bool isSceneLoaded() const;

    /**
     * @brief Check whether the user closed the window
     */
    bool shouldClose() const;

    /**
     * @brief Get the next key pressed in the window
     *
     * @return a lowercase letter or digit, 27 for Esc, or -1 if no key is waiting
     */
    int nextKey();

    /**
     * @brief Draw a frame and show it
     *
     * @param[in]  background  Camera image (CV_8UC3)
     * @param[in]  overlay     Overlay of the same size (CV_8UC4, premultiplied alpha)
     * @param[in]  projection  OpenGL projection matrix of the camera
     * @param[in]  view        OpenGL view matrix of the board, or null to skip the scene
     * @param[out] screenshot  If not null, receives the rendered frame (CV_8UC3)
     */
    void render(const cv::Mat &background, const cv::Mat &overlay, const glm::mat4 &projection,
                const glm::mat4 *view, cv::Mat *screenshot = nullptr);

private:
    /**
     * Owns the GLFW window. Declared first so it is destroyed last, after every member that
     * releases OpenGL objects.
     */
    struct Window
    {
        GLFWwindow *handle = nullptr;
        bool glfw_initialized = false;
        ~Window();
    };

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void drawImage(const gl::Texture &texture) const;

    Window window_;
    cv::Size frame_size_;
    gl::ShaderProgram image_program_;
    gl::Texture background_;
    gl::Texture overlay_;
    GLuint quad_vao_ = 0;
    SceneRenderer scene_;
    std::deque<int> keys_;
};

#endif
