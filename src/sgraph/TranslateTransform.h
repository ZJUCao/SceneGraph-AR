/**
 * @file TranslateTransform.h
 *
 * @brief The TranslateTransform class
 *
 * This node represents a specific kind of transform: translation.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_TRANSLATETRANSFORM_H
#define SGRAPH_TRANSLATETRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "sgraph/TransformNode.h"

namespace sgraph
{
    /**
     * This node represents a specific kind of transform: translation
     */
    class TranslateTransform : public TransformNode
    {
    public:
        TranslateTransform(float tx, float ty, float tz, const std::string &name,
                           IScenegraph *graph)
            : TransformNode(name, graph), translation(tx, ty, tz)
        {
            setTransform(glm::translate(glm::mat4(1.0f), translation));
        }

        /**
         * Visit this node
         *
         * @param visitor the visitor
         */
        void accept(SGNodeVisitor *visitor) override
        {
            visitor->visitTranslateTransform(this);
        }

        /**
         * Get the translation vector
         *
         * @return the translation vector
         */
        glm::vec3 getTranslate() const
        {
            return translation;
        }

    protected:
        glm::vec3 translation;

        ParentSGNode *copyNode() override
        {
            return new TranslateTransform(translation.x, translation.y, translation.z, name,
                                          scenegraph);
        }
    };
} // namespace sgraph

#endif
