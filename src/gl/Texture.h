/**
 * @file Texture.h
 *
 * @brief The gl::Texture class
 *
 * A 2D texture that is refilled from an OpenCV image every frame.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <glad/glad.h>
#include <opencv2/core.hpp>

namespace gl
{
    class Texture
    {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        /**
         * @brief Copy an image into the texture; needs a current OpenGL context
         *
         * @param[in] image  CV_8UC3 (BGR) or CV_8UC4 (BGRA) image; row 0 is the top row
         */
        void upload(const cv::Mat &image);

        /**
         * @brief Bind the texture to a texture unit
         */
        void bind(int unit) const;

    private:
        GLuint id_ = 0;
        int width_ = 0;
        int height_ = 0;
        int channels_ = 0;
    };
} // namespace gl

#endif
