/**
 * @file ARView.cpp
 *
 * @brief Implementation of the ARView class
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "ARView.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <opencv2/core.hpp>
#include <stdexcept>

namespace
{
    void printGlfwError(int code, const char *description)
    {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
    }
} // namespace

ARView::Window::~Window()
{
    if (handle != nullptr)
    {
        glfwDestroyWindow(handle);
    }
    if (glfw_initialized)
    {
        glfwTerminate();
    }
}

ARView::~ARView()
{
    // the other OpenGL objects are released by their members, before window_ is destroyed
    if (quad_vao_ != 0)
    {
        glDeleteVertexArrays(1, &quad_vao_);
    }
}

void ARView::open(cv::Size frame_size, const std::string &title)
{
    frame_size_ = frame_size;

    glfwSetErrorCallback(printGlfwError);
    if (!glfwInit())
    {
        throw std::runtime_error("Cannot initialize GLFW");
    }
    window_.glfw_initialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // required on macOS
    glfwWindowHint(GLFW_SAMPLES, 4);

    // start at the video's size, but fit on the screen
    double scale = 1.0;
    int area_x = 0, area_y = 0, area_w = 0, area_h = 0;
    if (GLFWmonitor *monitor = glfwGetPrimaryMonitor())
    {
        glfwGetMonitorWorkarea(monitor, &area_x, &area_y, &area_w, &area_h);
        if (area_w > 0 && area_h > 0)
        {
            scale =
                std::min({1.0, 0.9 * area_w / frame_size.width, 0.9 * area_h / frame_size.height});
        }
    }
    window_.handle = glfwCreateWindow(static_cast<int>(std::lround(frame_size.width * scale)),
                                      static_cast<int>(std::lround(frame_size.height * scale)),
                                      title.c_str(), nullptr, nullptr);
    if (window_.handle == nullptr)
    {
        throw std::runtime_error("Cannot create a window with an OpenGL 3.3 core context");
    }

    glfwMakeContextCurrent(window_.handle);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        throw std::runtime_error("Cannot load the OpenGL functions");
    }
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(window_.handle, this);
    glfwSetKeyCallback(window_.handle, keyCallback);

    image_program_.load("shaders/image.vert", "shaders/image.frag");
    image_program_.use();
    image_program_.setInt("image", 0);

    // the image quad has no vertex data, but core profile needs a vertex array bound
    glGenVertexArrays(1, &quad_vao_);
}

void ARView::loadScene(const std::string &scene_file, const glm::mat4 &placement)
{
    scene_.load(scene_file, placement);
}

bool ARView::isSceneLoaded() const
{
    return scene_.isLoaded();
}

bool ARView::shouldClose() const
{
    return window_.handle == nullptr || glfwWindowShouldClose(window_.handle);
}

int ARView::nextKey()
{
    if (keys_.empty())
    {
        return -1;
    }
    int key = keys_.front();
    keys_.pop_front();
    return key;
}

void ARView::keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (action != GLFW_PRESS)
    {
        return;
    }
    ARView *view = static_cast<ARView *>(glfwGetWindowUserPointer(window));
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
    {
        view->keys_.push_back('a' + (key - GLFW_KEY_A));
    }
    else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
    {
        view->keys_.push_back('0' + (key - GLFW_KEY_0));
    }
    else if (key == GLFW_KEY_ESCAPE)
    {
        view->keys_.push_back(27);
    }
}

void ARView::drawImage(const gl::Texture &texture) const
{
    image_program_.use();
    texture.bind(0);
    glBindVertexArray(quad_vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void ARView::render(const cv::Mat &background, const cv::Mat &overlay, const glm::mat4 &projection,
                    const glm::mat4 *view, cv::Mat *screenshot)
{
    // letterbox: the largest viewport with the video's aspect ratio
    int fb_width = 0, fb_height = 0;
    glfwGetFramebufferSize(window_.handle, &fb_width, &fb_height);
    double scale = std::min(static_cast<double>(fb_width) / frame_size_.width,
                            static_cast<double>(fb_height) / frame_size_.height);
    int vp_width = static_cast<int>(std::lround(frame_size_.width * scale));
    int vp_height = static_cast<int>(std::lround(frame_size_.height * scale));
    int vp_x = (fb_width - vp_width) / 2;
    int vp_y = (fb_height - vp_height) / 2;

    glViewport(0, 0, fb_width, fb_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(vp_x, vp_y, vp_width, vp_height);

    // 1. camera image, without touching the depth buffer
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    background_.upload(background);
    drawImage(background_);

    // 2. scene graph
    if (view != nullptr && scene_.isLoaded())
    {
        glEnable(GL_DEPTH_TEST);
        scene_.draw(projection, *view);
        glDisable(GL_DEPTH_TEST);
    }

    // 3. OpenCV overlay, premultiplied alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    overlay_.upload(overlay);
    drawImage(overlay_);
    glDisable(GL_BLEND);

    if (screenshot != nullptr)
    {
        cv::Mat pixels(vp_height, vp_width, CV_8UC3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(vp_x, vp_y, vp_width, vp_height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data);
        cv::flip(pixels, *screenshot, 0); // OpenGL rows start at the bottom
    }

    glfwSwapBuffers(window_.handle);
    glfwPollEvents();
}
