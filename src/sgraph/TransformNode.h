/**
 * @file TransformNode.h
 *
 * @brief The TransformNode class
 *
 * This node represents a transformation in the scene graph. It has only one child. The
 * transformation can be viewed as changing from its child's coordinate system to its parent's
 * coordinate system.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_TRANSFORMNODE_H
#define SGRAPH_TRANSFORMNODE_H

#include <glm/glm.hpp>
#include <stdexcept>
#include <string>
#include "sgraph/ParentSGNode.h"
#include "sgraph/SGNodeVisitor.h"

namespace sgraph
{
    /**
     * This node represents a transformation in the scene graph.
     */
    class TransformNode : public ParentSGNode
    {
    public:
        TransformNode(const std::string &name, IScenegraph *graph)
            : ParentSGNode(name, graph), transform(1.0f)
        {
        }

        /**
         * Adds the child to this node
         *
         * @param child the child of this node
         * @throws std::runtime_error if a child already exists
         */
        void addChild(SGNode *child) override
        {
            if (!children.empty())
            {
                throw std::runtime_error("Transform node already has a child");
            }
            children.push_back(child);
            child->setParent(this);
        }

        /**
         * Gets the transform at this node
         *
         * @return the transform at this node
         */
        const glm::mat4 &getTransform() const
        {
            return transform;
        }

        /**
         * Sets the scene graph object of which this node is a part, and then recurses to its
         * child
         *
         * @param graph a reference to the scene graph object of which this tree is a part
         */
        void setScenegraph(IScenegraph *graph) override
        {
            AbstractSGNode::setScenegraph(graph);
            if (!children.empty())
            {
                children[0]->setScenegraph(graph);
            }
        }

        /**
         * Visit this node
         *
         * @param visitor the visitor object
         */
        void accept(SGNodeVisitor *visitor) override
        {
            visitor->visitTransformNode(this);
        }

    protected:
        glm::mat4 transform;

        void setTransform(const glm::mat4 &matrix)
        {
            transform = matrix;
        }
    };
} // namespace sgraph

#endif
