/**
 * @file LeafNode.h
 *
 * @brief The LeafNode class
 *
 * This node represents the leaf of a scene graph. It is the only type of node that has actual
 * geometry to render: a reference to a mesh and a material.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_LEAFNODE_H
#define SGRAPH_LEAFNODE_H

#include <string>
#include "sgraph/AbstractSGNode.h"
#include "sgraph/SGNodeVisitor.h"
#include "util/Material.h"

namespace sgraph
{
    /**
     * This class implements the sgraph::SGNode interface for leaves.
     */
    class LeafNode : public AbstractSGNode
    {
    public:
        LeafNode(const std::string &instanceOf, const util::Material &material,
                 const std::string &name, IScenegraph *graph)
            : AbstractSGNode(name, graph), objInstanceName(instanceOf), material(material)
        {
        }

        LeafNode(const std::string &instanceOf, const std::string &name, IScenegraph *graph)
            : AbstractSGNode(name, graph), objInstanceName(instanceOf)
        {
        }

        /**
         * Set the material of this leaf
         *
         * @param mat the material
         */
        void setMaterial(const util::Material &mat)
        {
            material = mat;
        }

        /**
         * Get the material of this leaf
         *
         * @return the material
         */
        const util::Material &getMaterial() const
        {
            return material;
        }

        /**
         * Get the name of the mesh instance this leaf contains
         *
         * @return the instance name
         */
        const std::string &getInstanceOf() const
        {
            return objInstanceName;
        }

        /**
         * Get a copy of this node
         *
         * @return the copy
         */
        SGNode *clone() override
        {
            return new LeafNode(objInstanceName, material, name, scenegraph);
        }

        /**
         * Visit this node
         *
         * @param visitor the visitor object
         */
        void accept(SGNodeVisitor *visitor) override
        {
            visitor->visitLeafNode(this);
        }

    protected:
        std::string objInstanceName; // the name of the mesh instance that this leaf contains
        util::Material material;     // the material of the mesh instance at this leaf
    };
} // namespace sgraph

#endif
