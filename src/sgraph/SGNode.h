/**
 * @file SGNode.h
 *
 * @brief The SGNode interface
 *
 * This interface represents all the operations offered by each type of node in our scene graph.
 * Individual nodes may contain other functions unique to them.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_SGNODE_H
#define SGRAPH_SGNODE_H

#include <string>

namespace sgraph
{
    class IScenegraph;
    class SGNodeVisitor;

    /**
     * This interface represents all the operations offered by each type of node in our scene
     * graph. Individual nodes may contain other functions unique to them. Not all types of nodes
     * are able to offer all types of operations.
     */
    class SGNode
    {
    public:
        virtual ~SGNode() {}

        /**
         * In the scene graph rooted at this node, get the node whose name is as given
         *
         * @param name name of node to be searched
         * @return the node reference if it exists, null otherwise
         */
        virtual SGNode *getNode(const std::string &name) = 0;

        /**
         * Return a deep copy of the scene graph subtree rooted at this node
         *
         * @return a reference to the root of the copied subtree
         */
        virtual SGNode *clone() = 0;

        /**
         * Set the parent of this node. Each node except the root has a parent
         *
         * @param parent the node that is to be the parent of this node
         */
        virtual void setParent(SGNode *parent) = 0;

        /**
         * Traverse the scene graph rooted at this node, and store references to the scene graph
         * object
         *
         * @param graph a reference to the scene graph object of which this tree is a part
         */
        virtual void setScenegraph(IScenegraph *graph) = 0;

        /**
         * Set the name of this node. The name is not guaranteed to be unique in the tree, but it
         * should be.
         *
         * @param name the name of this node
         */
        virtual void setName(const std::string &name) = 0;

        /**
         * Get the name of this node
         *
         * @return the name of this node
         */
        virtual std::string getName() const = 0;

        /**
         * Accept a visitor to visit this node
         *
         * @param visitor the visitor to visit this node
         */
        virtual void accept(SGNodeVisitor *visitor) = 0;
    };
} // namespace sgraph

#endif
