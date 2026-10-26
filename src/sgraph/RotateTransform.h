/**
 * @file RotateTransform.h
 *
 * @brief The RotateTransform class
 *
 * This node represents a specific kind of transform: rotation about an axis.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_ROTATETRANSFORM_H
#define SGRAPH_ROTATETRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "sgraph/TransformNode.h"

namespace sgraph
{
    /**
     * This class represents a specific type of transformation: rotation
     */
    class RotateTransform : public TransformNode
    {
    public:
        RotateTransform(float angleInRadians, float ax, float ay, float az, const std::string &name,
                        IScenegraph *graph)
            : TransformNode(name, graph), angleInRadians(angleInRadians), axis(ax, ay, az)
        {
            setTransform(glm::rotate(glm::mat4(1.0f), angleInRadians, axis));
        }

        /**
         * Visit this node
         *
         * @param visitor the visitor
         */
        void accept(SGNodeVisitor *visitor) override
        {
            visitor->visitRotateTransform(this);
        }

        /**
         * Get the rotation axis
         *
         * @return the rotation axis
         */
        glm::vec3 getRotationAxis() const
        {
            return axis;
        }

        /**
         * Get the angle in radians
         *
         * @return the angle in radians
         */
        float getAngleInRadians() const
        {
            return angleInRadians;
        }

    protected:
        float angleInRadians;
        glm::vec3 axis;

        ParentSGNode *copyNode() override
        {
            return new RotateTransform(angleInRadians, axis.x, axis.y, axis.z, name, scenegraph);
        }
    };
} // namespace sgraph

#endif
