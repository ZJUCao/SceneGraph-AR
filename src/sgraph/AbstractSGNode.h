/**
 * @file AbstractSGNode.h
 *
 * @brief The AbstractSGNode class
 *
 * This abstract class implements the parts of the SGNode interface that are common to all
 * nodes: the name, the parent and the owning scene graph.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_ABSTRACTSGNODE_H
#define SGRAPH_ABSTRACTSGNODE_H

#include <string>
#include "sgraph/IScenegraph.h"
#include "sgraph/SGNode.h"

namespace sgraph
{
    /**
     * This abstract class implements the sgraph::SGNode interface.
     */
    class AbstractSGNode : public SGNode
    {
    public:
        AbstractSGNode(const std::string &name, IScenegraph *graph)
            : name(name), parent(nullptr), scenegraph(graph)
        {
        }

        /**
         * By default, this method checks only itself. Nodes that have children should override
         * this method and navigate to children to find the one with the correct name
         *
         * @param nodeName name of node to be searched
         * @return the node whose name this is, null otherwise
         */
        SGNode *getNode(const std::string &nodeName) override
        {
            return (name == nodeName) ? this : nullptr;
        }

        /**
         * Sets the parent of this node
         *
         * @param parentNode the node that is to be the parent of this node
         */
        void setParent(SGNode *parentNode) override
        {
            parent = parentNode;
        }

        /**
         * Sets the scene graph object whose part this node is and then adds itself to the scene
         * graph (in case the scene graph ever needs to directly access this node)
         *
         * @param graph a reference to the scene graph object of which this tree is a part
         */
        void setScenegraph(IScenegraph *graph) override
        {
            scenegraph = graph;
            graph->addNode(name, this);
        }

        /**
         * Sets the name of this node
         *
         * @param newName the name of this node
         */
        void setName(const std::string &newName) override
        {
            name = newName;
        }

        /**
         * Gets the name of this node
         *
         * @return the name of this node
         */
        std::string getName() const override
        {
            return name;
        }

    protected:
        std::string name;        // the name given to this node
        SGNode *parent;          // the parent of this node
        IScenegraph *scenegraph; // the scene graph that this node is part of
    };
} // namespace sgraph

#endif
