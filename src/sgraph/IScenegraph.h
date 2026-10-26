/**
 * @file IScenegraph.h
 *
 * @brief The IScenegraph interface
 *
 * This virtual class captures all the operations that a scene graph should offer. It is
 * designed to be a generic scene graph that is independent of the actual rendering library.
 *
 * The scene graph also stores references to all the nodes keyed by their name. This way the
 * scene graph can directly refer to any of its nodes by name instead of traversing the tree
 * every time to find it.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_ISCENEGRAPH_H
#define SGRAPH_ISCENEGRAPH_H

#include <map>
#include <string>
#include "VertexAttrib.h"
#include "sgraph/SGNode.h"
#include "util/PolygonMesh.h"

namespace sgraph
{
    /**
     * This virtual class captures all the operations that a scene graph should offer.
     */
    class IScenegraph
    {
    public:
        virtual ~IScenegraph() {}

        /**
         * Initialize the supplied root to be the root of this scene graph. This is supposed to
         * overwrite any previous roots
         *
         * @param root the root node of the scene graph
         */
        virtual void makeScenegraph(SGNode *root) = 0;

        /**
         * Adds a node to itself. This should be stored in a suitable manner by an
         * implementation, so that it is possible to look up a specific node by name
         *
         * @param name (hopefully unique) name given to this node
         * @param node the node object
         */
        virtual void addNode(const std::string &name, SGNode *node) = 0;

        /**
         * Get the root of this scene graph
         *
         * @return the root of this scene graph
         */
        virtual SGNode *getRoot() = 0;

        /**
         * Get a mapping of all (name, SGNode) pairs for all nodes in this scene graph
         *
         * @return a mapping of all (name, SGNode) pairs for all nodes in this scene graph
         */
        virtual const std::map<std::string, SGNode *> &getNodes() const = 0;

        /**
         * Delete the tree rooted at the root of this scene graph
         */
        virtual void dispose() = 0;

        /**
         * Set the meshes used by this scene graph
         *
         * @param meshes a mapping of all (mesh name, mesh object) pairs
         */
        virtual void setMeshes(
            const std::map<std::string, util::PolygonMesh<VertexAttrib>> &meshes) = 0;

        /**
         * Set the mesh name -> mesh path for all meshes used by this scene graph
         *
         * @param meshPaths a mapping of all (mesh name, mesh path) pairs
         */
        virtual void setMeshPaths(const std::map<std::string, std::string> &meshPaths) = 0;

        /**
         * Get the meshes used by this scene graph
         *
         * @return a mapping of all (mesh name, mesh object) pairs
         */
        virtual const std::map<std::string, util::PolygonMesh<VertexAttrib>> &getMeshes() const = 0;

        /**
         * Get a map of each mesh name (as the leaves refer to it) and the path to the mesh file
         *
         * @return a mapping of all (mesh name, mesh path) pairs
         */
        virtual const std::map<std::string, std::string> &getMeshPaths() const = 0;
    };
} // namespace sgraph

#endif
