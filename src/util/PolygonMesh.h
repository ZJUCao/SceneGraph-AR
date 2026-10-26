/**
 * @file PolygonMesh.h
 *
 * @brief The PolygonMesh class
 *
 * A polygon mesh that works with any representation of vertex attributes implementing the
 * IVertexData interface.
 *
 * @author Amit Shesh
 */
#ifndef UTIL_POLYGONMESH_H
#define UTIL_POLYGONMESH_H

#include <glm/glm.hpp>
#include <vector>

namespace util
{
    /**
     * This class represents a polygon mesh. It stores:
     *
     * - A list of vertices: each vertex is an object that stores attributes of that vertex like
     *   position, normal, texture coordinates and others.
     * - A list of indices into the above array. This indexed representation allows vertices to
     *   be shared between polygons efficiently.
     * - How to interpret the indices (e.g. read 3 at a time to form a triangle).
     * - How many indices make a polygon (2 for line, 3 for triangle, 4 for quad, etc.)
     */
    template <class VertexType>
    class PolygonMesh
    {
    public:
        PolygonMesh() : primitiveType(0), primitiveSize(0), minBounds(0.0f), maxBounds(0.0f) {}

        /**
         * Set the primitive type. The primitive type is represented by an integer, for example
         * the value of GL_TRIANGLES.
         */
        void setPrimitiveType(int v)
        {
            primitiveType = v;
        }

        int getPrimitiveType() const
        {
            return primitiveType;
        }

        /**
         * Set how many indices make up a primitive.
         */
        void setPrimitiveSize(int s)
        {
            primitiveSize = s;
        }

        int getPrimitiveSize() const
        {
            return primitiveSize;
        }

        int getPrimitiveCount() const
        {
            return static_cast<int>(primitives.size());
        }

        int getVertexCount() const
        {
            return static_cast<int>(vertexData.size());
        }

        glm::vec4 getMinimumBounds() const
        {
            return minBounds;
        }

        glm::vec4 getMaximumBounds() const
        {
            return maxBounds;
        }

        const std::vector<VertexType> &getVertexAttributes() const
        {
            return vertexData;
        }

        const std::vector<unsigned int> &getPrimitives() const
        {
            return primitives;
        }

        void setVertexData(const std::vector<VertexType> &vp)
        {
            vertexData = vp;
            computeBoundingBox();
        }

        void setPrimitives(const std::vector<unsigned int> &t)
        {
            primitives = t;
        }

        /**
         * Compute the bounding box of this polygon mesh, if there is position data
         */
        void computeBoundingBox()
        {
            if (vertexData.empty() || !vertexData[0].hasData("position"))
            {
                return;
            }

            for (size_t i = 0; i < vertexData.size(); i++)
            {
                std::vector<float> data = vertexData[i].getData("position");
                glm::vec4 pos(0, 0, 0, 1);
                for (size_t j = 0; j < data.size() && j < 4; j++)
                {
                    pos[static_cast<glm::length_t>(j)] = data[j];
                }

                if (i == 0)
                {
                    minBounds = maxBounds = pos;
                }
                else
                {
                    minBounds = glm::min(minBounds, pos);
                    maxBounds = glm::max(maxBounds, pos);
                }
            }
        }

    protected:
        std::vector<VertexType> vertexData;
        std::vector<unsigned int> primitives;
        int primitiveType;
        int primitiveSize;
        glm::vec4 minBounds, maxBounds; // bounding box
    };
} // namespace util

#endif
