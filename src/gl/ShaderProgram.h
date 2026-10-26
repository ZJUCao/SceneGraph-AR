/**
 * @file ShaderProgram.h
 *
 * @brief The gl::ShaderProgram class
 *
 * A GLSL program built from a vertex and a fragment shader file, with uniform setters that
 * look up (and cache) locations by name.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef GL_SHADERPROGRAM_H
#define GL_SHADERPROGRAM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

namespace gl
{
    class ShaderProgram
    {
    public:
        ShaderProgram() = default;
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram &) = delete;
        ShaderProgram &operator=(const ShaderProgram &) = delete;

        /**
         * @brief Compile and link the program; needs a current OpenGL context
         *
         * @param[in] vertex_file    Path of the vertex shader source
         * @param[in] fragment_file  Path of the fragment shader source
         * @throws std::runtime_error with the compiler or linker log on failure
         */
        void load(const std::string &vertex_file, const std::string &fragment_file);

        /**
         * @brief Make this the current program
         */
        void use() const;

        void setInt(const std::string &name, int value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setMat3(const std::string &name, const glm::mat3 &value) const;
        void setMat4(const std::string &name, const glm::mat4 &value) const;

    private:
        GLint location(const std::string &name) const;

        GLuint id_ = 0;
        mutable std::map<std::string, GLint> locations_;
    };
} // namespace gl

#endif
