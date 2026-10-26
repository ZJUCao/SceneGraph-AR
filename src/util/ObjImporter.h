/**
 * @file ObjImporter.h
 *
 * @brief The ObjImporter class
 *
 * A helper class to import a PolygonMesh object from a Wavefront OBJ file.
 *
 * @author Amit Shesh
 */
#ifndef UTIL_OBJIMPORTER_H
#define UTIL_OBJIMPORTER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdlib>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "util/PolygonMesh.h"

namespace util
{
    /**
     * A helper class to import a PolygonMesh object from an OBJ file. It imports only position,
     * normal and texture coordinate data (if present). Polygons with more than three vertices
     * are split into triangle fans.
     */
    template <class K>
    class ObjImporter
    {
    public:
        /**
         * Primitive type stored in imported meshes; the same value as GL_TRIANGLES, so meshes
         * stay compatible with OpenGL code without this project depending on OpenGL.
         */
        static const int TRIANGLES = 0x0004;

        /**
         * Read a mesh from an OBJ stream
         *
         * @param in              the OBJ file contents
         * @param scaleAndCenter  if true, center the mesh at the origin and scale it to fit in a
         *                        unit cube
         * @return the triangle mesh
         * @throws std::runtime_error if the file is malformed
         */
        static PolygonMesh<K> importFile(std::istream &in, bool scaleAndCenter)
        {
            std::vector<glm::vec4> vertices, normals, texcoords;
            std::vector<unsigned int> triangles;
            int lineno = 0;
            std::string line;

            while (std::getline(in, line))
            {
                lineno++;

                std::istringstream str(line);
                std::vector<std::string> tokens;
                std::string symbol;
                while (str >> symbol)
                {
                    tokens.push_back(symbol);
                }

                // skip blank lines and comments
                if (tokens.empty() || tokens[0][0] == '#')
                {
                    continue;
                }

                if (tokens[0] == "v")
                {
                    if ((tokens.size() < 4) || (tokens.size() > 7))
                    {
                        throw error(lineno, "Vertex coordinate has an invalid number of values");
                    }

                    glm::vec4 v(toFloat(tokens[1]), toFloat(tokens[2]), toFloat(tokens[3]), 1.0f);
                    if (tokens.size() == 5)
                    {
                        float w = toFloat(tokens[4]);
                        if (w != 0)
                        {
                            v.x /= w;
                            v.y /= w;
                            v.z /= w;
                        }
                    }
                    vertices.push_back(v);
                }
                else if (tokens[0] == "vt")
                {
                    if ((tokens.size() < 3) || (tokens.size() > 4))
                    {
                        throw error(lineno, "Texture coordinate has an invalid number of values");
                    }

                    glm::vec4 v(toFloat(tokens[1]), toFloat(tokens[2]), 0.0f, 1.0f);
                    if (tokens.size() > 3)
                    {
                        v.z = toFloat(tokens[3]);
                    }
                    texcoords.push_back(v);
                }
                else if (tokens[0] == "vn")
                {
                    if (tokens.size() != 4)
                    {
                        throw error(lineno, "Normal has an invalid number of values");
                    }

                    glm::vec3 n(toFloat(tokens[1]), toFloat(tokens[2]), toFloat(tokens[3]));
                    normals.push_back(glm::vec4(glm::normalize(n), 0.0f));
                }
                else if (tokens[0] == "f")
                {
                    std::vector<unsigned int> polygon;
                    for (size_t i = 1; i < tokens.size(); i++)
                    {
                        // each vertex is "v", "v/vt", "v//vn" or "v/vt/vn"; only v is used for
                        // indexing. OBJ indices begin at 1.
                        std::string index = tokens[i].substr(0, tokens[i].find('/'));
                        int vi = std::atoi(index.c_str());
                        if (vi < 1)
                        {
                            throw error(lineno, "Face specification has an invalid vertex index");
                        }
                        polygon.push_back(static_cast<unsigned int>(vi - 1));
                    }

                    if (polygon.size() < 3)
                    {
                        throw error(lineno, "Fewer than 3 vertices for a polygon");
                    }

                    // if the face has more than 3 vertices, break it down into a triangle fan
                    for (size_t i = 2; i < polygon.size(); i++)
                    {
                        triangles.push_back(polygon[0]);
                        triangles.push_back(polygon[i - 1]);
                        triangles.push_back(polygon[i]);
                    }
                }
            }

            if (scaleAndCenter && !vertices.empty())
            {
                // center about the origin and fit within a cube of side 1
                glm::vec4 minimum = vertices[0];
                glm::vec4 maximum = vertices[0];
                for (const glm::vec4 &v : vertices)
                {
                    minimum = glm::min(minimum, v);
                    maximum = glm::max(maximum, v);
                }

                glm::vec4 center = (minimum + maximum) * 0.5f;
                float longest = std::max(maximum.x - minimum.x,
                                         std::max(maximum.y - minimum.y, maximum.z - minimum.z));

                // first translate and then scale
                glm::mat4 transform =
                    glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / longest)) *
                    glm::translate(glm::mat4(1.0f), glm::vec3(-center.x, -center.y, -center.z));
                for (glm::vec4 &v : vertices)
                {
                    v = transform * v;
                }
            }

            std::vector<K> vertexData;
            for (size_t i = 0; i < vertices.size(); i++)
            {
                K v;
                v.setData("position", toVector(vertices[i]));
                if (texcoords.size() == vertices.size())
                {
                    v.setData("texcoord", toVector(texcoords[i]));
                }
                if (normals.size() == vertices.size())
                {
                    v.setData("normal", toVector(normals[i]));
                }
                vertexData.push_back(v);
            }

            PolygonMesh<K> mesh;
            mesh.setVertexData(vertexData);
            mesh.setPrimitives(triangles);
            mesh.setPrimitiveType(TRIANGLES);
            mesh.setPrimitiveSize(3);
            return mesh;
        }

    private:
        static float toFloat(const std::string &token)
        {
            return static_cast<float>(std::atof(token.c_str()));
        }

        static std::vector<float> toVector(const glm::vec4 &v)
        {
            return {v.x, v.y, v.z, v.w};
        }

        static std::runtime_error error(int lineno, const std::string &message)
        {
            return std::runtime_error("OBJ line " + std::to_string(lineno) + ": " + message);
        }
    };
} // namespace util

#endif
