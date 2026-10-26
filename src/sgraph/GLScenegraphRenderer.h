/**
 * @file GLScenegraphRenderer.h
 *
 * @brief The GLScenegraphRenderer class
 *
 * A scene graph visitor that draws the scene with OpenGL. Transform nodes push their matrix
 * onto a modelview stack; every leaf draws its mesh with the current modelview matrix, the
 * matching normal matrix and its material color.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef SGRAPH_GLSCENEGRAPHRENDERER_H
#define SGRAPH_GLSCENEGRAPHRENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include "gl/Mesh.h"
#include "gl/ShaderProgram.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/RotateTransform.h"
#include "sgraph/SGNodeVisitor.h"
#include "sgraph/ScaleTransform.h"
#include "sgraph/TransformNode.h"
#include "sgraph/TranslateTransform.h"

namespace sgraph
{
    class GLScenegraphRenderer : public SGNodeVisitor
    {
    public:
        /**
         * @param modelview  matrix stack; its top is the world-to-eye (view) matrix
         * @param meshes     GPU meshes by the names leaves refer to
         * @param program    the enabled shader program
         */
        GLScenegraphRenderer(std::stack<glm::mat4> &modelview,
                             const std::map<std::string, std::unique_ptr<gl::Mesh>> &meshes,
                             const gl::ShaderProgram &program)
            : modelview(modelview), meshes(meshes), program(program)
        {
        }

        void visitGroupNode(GroupNode *groupNode) override
        {
            for (SGNode *child : groupNode->getChildren())
            {
                child->accept(this);
            }
        }

        void visitLeafNode(LeafNode *leafNode) override
        {
            auto it = meshes.find(leafNode->getInstanceOf());
            if (it == meshes.end())
            {
                return;
            }

            // the scene files give most materials only an ambient color; use the diffuse
            // color when there is one
            const util::Material &material = leafNode->getMaterial();
            glm::vec3 color(material.getDiffuse());
            if (glm::length(color) == 0.0f)
            {
                color = glm::vec3(material.getAmbient());
            }

            program.setMat4("modelview", modelview.top());
            program.setMat3("normalmatrix", glm::inverseTranspose(glm::mat3(modelview.top())));
            program.setVec3("materialColor", color);
            it->second->draw();
            trianglesDrawn += it->second->triangleCount();
        }

        void visitTransformNode(TransformNode *transformNode) override
        {
            modelview.push(modelview.top() * transformNode->getTransform());
            if (!transformNode->getChildren().empty())
            {
                transformNode->getChildren()[0]->accept(this);
            }
            modelview.pop();
        }

        void visitScaleTransform(ScaleTransform *scaleNode) override
        {
            visitTransformNode(scaleNode);
        }

        void visitTranslateTransform(TranslateTransform *translateNode) override
        {
            visitTransformNode(translateNode);
        }

        void visitRotateTransform(RotateTransform *rotateNode) override
        {
            visitTransformNode(rotateNode);
        }

        /**
         * @brief Get the number of triangles drawn so far
         */
        size_t getTrianglesDrawn() const
        {
            return trianglesDrawn;
        }

    private:
        std::stack<glm::mat4> &modelview;
        const std::map<std::string, std::unique_ptr<gl::Mesh>> &meshes;
        const gl::ShaderProgram &program;
        size_t trianglesDrawn = 0;
    };
} // namespace sgraph

#endif
