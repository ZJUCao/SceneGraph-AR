/**
 * @file SGNodeVisitor.h
 *
 * @brief The SGNodeVisitor interface
 *
 * The visitor used by the scene graph nodes. The visitor design pattern lets one implement
 * various operations on the scene graph without having to change the nodes.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_SGNODEVISITOR_H
#define SGRAPH_SGNODEVISITOR_H

namespace sgraph
{
    class GroupNode;
    class LeafNode;
    class TransformNode;
    class ScaleTransform;
    class RotateTransform;
    class TranslateTransform;

    /**
     * This class represents the interface for a visitor on scene graph nodes.
     */
    class SGNodeVisitor
    {
    public:
        virtual ~SGNodeVisitor() {}

        virtual void visitGroupNode(GroupNode *node) = 0;
        virtual void visitLeafNode(LeafNode *node) = 0;
        virtual void visitTransformNode(TransformNode *node) = 0;
        virtual void visitScaleTransform(ScaleTransform *node) = 0;
        virtual void visitTranslateTransform(TranslateTransform *node) = 0;
        virtual void visitRotateTransform(RotateTransform *node) = 0;
    };
} // namespace sgraph

#endif
