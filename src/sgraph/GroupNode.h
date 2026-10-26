/**
 * @file GroupNode.h
 *
 * @brief The GroupNode class
 *
 * A group node is simply a logical grouping of other nodes. It can have an arbitrary number of
 * children, of any type.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_GROUPNODE_H
#define SGRAPH_GROUPNODE_H

#include <string>
#include "sgraph/ParentSGNode.h"
#include "sgraph/SGNodeVisitor.h"

namespace sgraph
{
    /**
     * This class represents a group node in the scene graph.
     */
    class GroupNode : public ParentSGNode
    {
    public:
        GroupNode(const std::string &name, IScenegraph *graph) : ParentSGNode(name, graph) {}

        /**
         * Sets the reference to the scene graph object for this node, and then recurses down to
         * children for the same
         *
         * @param graph a reference to the scene graph object of which this tree is a part
         */
        void setScenegraph(IScenegraph *graph) override
        {
            AbstractSGNode::setScenegraph(graph);
            for (SGNode *child : children)
            {
                child->setScenegraph(graph);
            }
        }

        /**
         * Adds a child to this node
         *
         * @param child the child node to be added
         */
        void addChild(SGNode *child) override
        {
            children.push_back(child);
            child->setParent(this);
        }

        /**
         * Visit this node
         *
         * @param visitor the visitor object
         */
        void accept(SGNodeVisitor *visitor) override
        {
            visitor->visitGroupNode(this);
        }

    protected:
        ParentSGNode *copyNode() override
        {
            return new GroupNode(name, scenegraph);
        }
    };
} // namespace sgraph

#endif
