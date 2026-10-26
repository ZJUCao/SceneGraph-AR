/**
 * @file Texture.cpp
 *
 * @brief Implementation of the gl::Texture class
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "gl/Texture.h"

#include <stdexcept>

namespace gl
{
    Texture::~Texture()
    {
        if (id_ != 0)
        {
            glDeleteTextures(1, &id_);
        }
    }

    void Texture::upload(const cv::Mat &image)
    {
        if (image.type() != CV_8UC3 && image.type() != CV_8UC4)
        {
            throw std::runtime_error("Texture::upload expects an 8-bit BGR or BGRA image");
        }
        cv::Mat pixels = image.isContinuous() ? image : image.clone();

        if (id_ == 0)
        {
            glGenTextures(1, &id_);
            glBindTexture(GL_TEXTURE_2D, id_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(GL_TEXTURE_2D, id_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // OpenCV stores pixels as BGR(A); the texture keeps them in RGB(A) order
        const int channels = pixels.channels();
        const GLenum format = (channels == 4) ? GL_BGRA : GL_BGR;
        if (pixels.cols == width_ && pixels.rows == height_ && channels == channels_)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixels.cols, pixels.rows, format,
                            GL_UNSIGNED_BYTE, pixels.data);
        }
        else
        {
            const GLint internal_format = (channels == 4) ? GL_RGBA8 : GL_RGB8;
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, pixels.cols, pixels.rows, 0, format,
                         GL_UNSIGNED_BYTE, pixels.data);
            width_ = pixels.cols;
            height_ = pixels.rows;
            channels_ = channels;
        }
    }

    void Texture::bind(int unit) const
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
        glBindTexture(GL_TEXTURE_2D, id_);
    }
} // namespace gl
