/**
 * @file SceneRenderer.h
 *
 * @brief The SceneRenderer class
 *
 * Draws a scene graph model with OpenGL so that it stands on the checkerboard: the scene is
 * loaded and its meshes are uploaded once, and every frame the graph is traversed with the
 * view matrix of the current board pose. Surfaces are lit by a directional light fixed to
 * the board, so the shading stays consistent as the camera moves.
 *
 * @author Yu Cao
 * @date 2026-10-25
 */
#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include "gl/Mesh.h"
#include "gl/ShaderProgram.h"
#include "sgraph/IScenegraph.h"

class SceneRenderer
{
public:
    SceneRenderer();
    ~SceneRenderer();

    /**
     * @brief Load a scene file, upload its meshes and compile the shaders; needs a current
     *        OpenGL context
     *
     * @param[in] scene_file  Path of the scene graph text file
     * @param[in] placement   Transform from scene coordinates to board coordinates
     *                        (one unit = one checkerboard square, Z out of the board)
     * @throws std::runtime_error if a file cannot be read, parsed or compiled
     */
    void load(const std::string &scene_file, const glm::mat4 &placement);

    /**
     * @brief Check whether a scene is loaded
     */
    bool isLoaded() const;

    /**
     * @brief Draw the scene; depth testing must be enabled by the caller
     *
     * @param[in] projection  OpenGL projection matrix of the camera
     * @param[in] view        OpenGL view matrix of the board pose
     * @return the number of triangles drawn
     */
    size_t draw(const glm::mat4 &projection, const glm::mat4 &view) const;

private:
    std::unique_ptr<sgraph::IScenegraph> scenegraph_;
    std::map<std::string, std::unique_ptr<gl::Mesh>> meshes_;
    gl::ShaderProgram program_;
    glm::mat4 placement_;
};

#endif
