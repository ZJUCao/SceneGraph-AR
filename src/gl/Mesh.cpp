/**
 * @file Mesh.cpp
 *
 * @brief Implementation of the gl::Mesh class
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "gl/Mesh.h"

#include <glm/glm.hpp>
#include <vector>

namespace gl
{
    Mesh::Mesh(const util::PolygonMesh<VertexAttrib> &mesh)
    {
        const std::vector<VertexAttrib> &vertices = mesh.getVertexAttributes();
        const std::vector<unsigned int> &indices = mesh.getPrimitives();

        std::vector<glm::vec4> positions, normals;
        bool has_normals = false;
        for (const VertexAttrib &vertex : vertices)
        {
            std::vector<float> p = vertex.getData("position");
            std::vector<float> n = vertex.getData("normal");
            positions.push_back(glm::vec4(p[0], p[1], p[2], p[3]));
            normals.push_back(glm::vec4(n[0], n[1], n[2], 0.0f));
            has_normals = has_normals || glm::length(glm::vec3(normals.back())) > 0.0f;
        }

        // without normals in the file, average the (area-weighted) normals of the triangles
        // that share each vertex
        if (!has_normals)
        {
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
            {
                glm::vec3 a(positions[indices[i]]), b(positions[indices[i + 1]]),
                    c(positions[indices[i + 2]]);
                glm::vec4 face_normal(glm::cross(b - a, c - a), 0.0f);
                for (size_t k = 0; k < 3; k++)
                {
                    normals[indices[i + k]] += face_normal;
                }
            }
            for (glm::vec4 &n : normals)
            {
                float length = glm::length(n);
                n = (length > 0.0f) ? n / length : n;
            }
        }

        std::vector<float> interleaved;
        interleaved.reserve(vertices.size() * 8);
        for (size_t i = 0; i < vertices.size(); i++)
        {
            for (int k = 0; k < 4; k++)
            {
                interleaved.push_back(positions[i][k]);
            }
            for (int k = 0; k < 4; k++)
            {
                interleaved.push_back(normals[i][k]);
            }
        }
        index_count_ = static_cast<GLsizei>(indices.size());

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(interleaved.size() * sizeof(float)),
                     interleaved.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(),
                     GL_STATIC_DRAW);

        const GLsizei stride = 8 * sizeof(float);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const void *>(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    Mesh::~Mesh()
    {
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
            glDeleteBuffers(1, &vbo_);
            glDeleteBuffers(1, &ebo_);
        }
    }

    void Mesh::draw() const
    {
        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    size_t Mesh::triangleCount() const
    {
        return static_cast<size_t>(index_count_) / 3;
    }
} // namespace gl
