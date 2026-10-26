/**
 * @file ScaleTransform.h
 *
 * @brief The ScaleTransform class
 *
 * This node represents a specific kind of transform: scale.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_SCALETRANSFORM_H
#define SGRAPH_SCALETRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "sgraph/TransformNode.h"

namespace sgraph
{
    /**
     * This node represents a specific type of transform: scale
     */
    class ScaleTransform : public TransformNode
    {
    public:
        ScaleTransform(float sx, float sy, float sz, const std::string &name, IScenegraph *graph)
            : TransformNode(name, graph), scale(sx, sy, sz)
        {
            setTransform(glm::scale(glm::mat4(1.0f), scale));
        }

        /**
         * Visit this node
         *
         * @param visitor the visitor
         */
        void accept(SGNodeVisitor *visitor) override
        {
            visitor->visitScaleTransform(this);
        }

        /**
         * Gets the scale at this node
         *
         * @return the scale at this node
         */
        glm::vec3 getScale() const
        {
            return scale;
        }

    protected:
        glm::vec3 scale;

        ParentSGNode *copyNode() override
        {
            return new ScaleTransform(scale.x, scale.y, scale.z, name, scenegraph);
        }
    };
} // namespace sgraph

#endif
