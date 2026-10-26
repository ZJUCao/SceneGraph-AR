/**
 * @file ParentSGNode.h
 *
 * @brief The ParentSGNode class
 *
 * This class represents an SGNode that can have children. It owns its children and deletes
 * them when it is deleted.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_PARENTSGNODE_H
#define SGRAPH_PARENTSGNODE_H

#include <string>
#include <vector>
#include "sgraph/AbstractSGNode.h"

namespace sgraph
{
    /**
     * This class represents an SGNode that can have children
     */
    class ParentSGNode : public AbstractSGNode
    {
    public:
        ParentSGNode(const std::string &name, IScenegraph *scenegraph)
            : AbstractSGNode(name, scenegraph)
        {
        }

        ~ParentSGNode() override
        {
            for (SGNode *child : children)
            {
                delete child;
            }
        }

        ParentSGNode(const ParentSGNode &) = delete;
        ParentSGNode &operator=(const ParentSGNode &) = delete;

        virtual void addChild(SGNode *child) = 0;

        const std::vector<SGNode *> &getChildren() const
        {
            return children;
        }

        /**
         * Searches recursively into its subtree to look for node with specified name
         *
         * @param nodeName name of node to be searched
         * @return the node whose name this is if it exists within this subtree, null otherwise
         */
        SGNode *getNode(const std::string &nodeName) override
        {
            SGNode *answer = AbstractSGNode::getNode(nodeName);
            for (size_t i = 0; (i < children.size()) && (answer == nullptr); i++)
            {
                answer = children[i]->getNode(nodeName);
            }
            return answer;
        }

        /**
         * Creates a deep copy of the subtree rooted at this node
         *
         * @return a deep copy of the subtree rooted at this node
         */
        SGNode *clone() override
        {
            ParentSGNode *copy = copyNode();
            for (SGNode *child : children)
            {
                copy->addChild(child->clone());
            }
            return copy;
        }

    protected:
        std::vector<SGNode *> children;

        /**
         * Create a node of the same type and parameters as this one, without children
         */
        virtual ParentSGNode *copyNode() = 0;
    };
} // namespace sgraph

#endif
