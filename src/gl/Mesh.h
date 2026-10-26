/**
 * @file Mesh.h
 *
 * @brief The gl::Mesh class
 *
 * A triangle mesh uploaded to the GPU once: a vertex array object with interleaved positions
 * (attribute 0) and normals (attribute 1), and an index buffer.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef GL_MESH_H
#define GL_MESH_H

#include <glad/glad.h>
#include <cstddef>
#include "VertexAttrib.h"
#include "util/PolygonMesh.h"

namespace gl
{
    class Mesh
    {
    public:
        /**
         * @brief Upload a triangle mesh; needs a current OpenGL context
         *
         * Vertex normals are computed from the triangles if the mesh has none.
         *
         * @param[in] mesh  The mesh, with primitive size 3
         */
        explicit Mesh(const util::PolygonMesh<VertexAttrib> &mesh);
        ~Mesh();

        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        /**
         * @brief Draw all triangles with the current program
         */
        void draw() const;

        size_t triangleCount() const;

    private:
        GLuint vao_ = 0;
        GLuint vbo_ = 0;
        GLuint ebo_ = 0;
        GLsizei index_count_ = 0;
    };
} // namespace gl

#endif
