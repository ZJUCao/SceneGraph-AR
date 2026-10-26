/**
 * @file Scenegraph.h
 *
 * @brief The Scenegraph class
 *
 * A specific implementation of the scene graph. This implementation is still independent of the
 * rendering technology.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_SCENEGRAPH_H
#define SGRAPH_SCENEGRAPH_H

#include <map>
#include <string>
#include "VertexAttrib.h"
#include "sgraph/IScenegraph.h"
#include "sgraph/SGNode.h"
#include "util/PolygonMesh.h"

namespace sgraph
{
    /**
     * A specific implementation of the scene graph. It owns the tree under its root.
     */
    class Scenegraph : public IScenegraph
    {
    public:
        Scenegraph() : root(nullptr) {}

        ~Scenegraph() override
        {
            dispose();
        }

        Scenegraph(const Scenegraph &) = delete;
        Scenegraph &operator=(const Scenegraph &) = delete;

        /**
         * @brief Delete the root node and all its children
         */
        void dispose() override
        {
            delete root;
            root = nullptr;
        }

        /**
         * @brief Set the root of the scene graph, and then pass a reference to this scene graph
         * object to all its nodes. This will enable any node to call functions of its associated
         * scene graph
         *
         * @param rootNode The root node of the scene graph
         */
        void makeScenegraph(SGNode *rootNode) override
        {
            root = rootNode;
            if (root != nullptr)
            {
                root->setScenegraph(this);
            }
        }

        /**
         * @brief Add a node to the scene graph
         *
         * @param name The name of the node
         * @param node The node to add
         */
        void addNode(const std::string &name, SGNode *node) override
        {
            nodes[name] = node;
        }

        SGNode *getRoot() override
        {
            return root;
        }

        const std::map<std::string, SGNode *> &getNodes() const override
        {
            return nodes;
        }

        void setMeshes(
            const std::map<std::string, util::PolygonMesh<VertexAttrib>> &newMeshes) override
        {
            meshes = newMeshes;
        }

        const std::map<std::string, util::PolygonMesh<VertexAttrib>> &getMeshes() const override
        {
            return meshes;
        }

        void setMeshPaths(const std::map<std::string, std::string> &newMeshPaths) override
        {
            meshPaths = newMeshPaths;
        }

        const std::map<std::string, std::string> &getMeshPaths() const override
        {
            return meshPaths;
        }

    protected:
        SGNode *root; // The root of the scene graph tree
        std::map<std::string, util::PolygonMesh<VertexAttrib>> meshes;
        std::map<std::string, std::string> meshPaths;
        std::map<std::string, SGNode *> nodes; // (name, node) pairs, for efficient search
    };
} // namespace sgraph

#endif
