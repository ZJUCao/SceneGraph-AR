/**
 * @file SceneRenderer.cpp
 *
 * @brief Implementation of the SceneRenderer class
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#include "SceneRenderer.h"

#include <fstream>
#include <stack>
#include <stdexcept>
#include "sgraph/GLScenegraphRenderer.h"
#include "sgraph/ScenegraphImporter.h"

namespace
{
    // direction toward the light in board coordinates: from above (+Z), slightly in front of
    // the board (-Y) and to the left (-X)
    const glm::vec3 LIGHT_DIRECTION(-0.4f, -0.6f, 1.0f);
} // namespace

SceneRenderer::SceneRenderer() : placement_(1.0f) {}

SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::load(const std::string &scene_file, const glm::mat4 &placement)
{
    std::ifstream in(scene_file);
    if (!in.is_open())
    {
        throw std::runtime_error("Cannot open scene file: " + scene_file);
    }

    sgraph::ScenegraphImporter importer;
    scenegraph_.reset(importer.parse(in));
    placement_ = placement;

    meshes_.clear();
    for (const auto &mesh : scenegraph_->getMeshes())
    {
        meshes_[mesh.first] = std::make_unique<gl::Mesh>(mesh.second);
    }

    program_.load("shaders/scene.vert", "shaders/scene.frag");
}

bool SceneRenderer::isLoaded() const
{
    return scenegraph_ != nullptr && scenegraph_->getRoot() != nullptr;
}

size_t SceneRenderer::draw(const glm::mat4 &projection, const glm::mat4 &view) const
{
    if (!isLoaded())
    {
        return 0;
    }

    program_.use();
    program_.setMat4("projection", projection);
    program_.setVec3("lightDirection",
                     glm::normalize(glm::vec3(view * glm::vec4(LIGHT_DIRECTION, 0.0f))));

    std::stack<glm::mat4> modelview;
    modelview.push(view * placement_);
    sgraph::GLScenegraphRenderer renderer(modelview, meshes_, program_);
    scenegraph_->getRoot()->accept(&renderer);
    return renderer.getTrianglesDrawn();
}
