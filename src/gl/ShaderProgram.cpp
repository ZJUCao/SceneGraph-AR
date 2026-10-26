/**
 * @file ShaderProgram.cpp
 *
 * @brief Implementation of the gl::ShaderProgram class
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "gl/ShaderProgram.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
    std::string readFile(const std::string &path)
    {
        std::ifstream in(path);
        if (!in.is_open())
        {
            throw std::runtime_error("Cannot open shader file: " + path);
        }
        std::ostringstream contents;
        contents << in.rdbuf();
        return contents.str();
    }

    GLuint compile(GLenum type, const std::string &path)
    {
        std::string source = readFile(path);
        const char *text = source.c_str();

        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &text, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (ok != GL_TRUE)
        {
            GLint length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(static_cast<size_t>(length) + 1, '\0');
            glGetShaderInfoLog(shader, length, nullptr, log.data());
            glDeleteShader(shader);
            throw std::runtime_error("Cannot compile " + path + ":\n" + log.data());
        }
        return shader;
    }
} // namespace

namespace gl
{
    ShaderProgram::~ShaderProgram()
    {
        if (id_ != 0)
        {
            glDeleteProgram(id_);
        }
    }

    void ShaderProgram::load(const std::string &vertex_file, const std::string &fragment_file)
    {
        GLuint vertex = compile(GL_VERTEX_SHADER, vertex_file);
        GLuint fragment = 0;
        try
        {
            fragment = compile(GL_FRAGMENT_SHADER, fragment_file);
        }
        catch (...)
        {
            glDeleteShader(vertex);
            throw;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        GLint ok = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (ok != GL_TRUE)
        {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(static_cast<size_t>(length) + 1, '\0');
            glGetProgramInfoLog(program, length, nullptr, log.data());
            glDeleteProgram(program);
            throw std::runtime_error("Cannot link " + vertex_file + " and " + fragment_file +
                                     ":\n" + log.data());
        }

        if (id_ != 0)
        {
            glDeleteProgram(id_);
        }
        id_ = program;
        locations_.clear();
    }

    void ShaderProgram::use() const
    {
        glUseProgram(id_);
    }

    void ShaderProgram::setInt(const std::string &name, int value) const
    {
        glUniform1i(location(name), value);
    }

    void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(location(name), 1, glm::value_ptr(value));
    }

    void ShaderProgram::setMat3(const std::string &name, const glm::mat3 &value) const
    {
        glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &value) const
    {
        glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    GLint ShaderProgram::location(const std::string &name) const
    {
        auto it = locations_.find(name);
        if (it == locations_.end())
        {
            it = locations_.emplace(name, glGetUniformLocation(id_, name.c_str())).first;
        }
        return it->second;
    }
} // namespace gl
