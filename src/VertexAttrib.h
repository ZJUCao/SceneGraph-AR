/**
 * @file VertexAttrib.h
 *
 * @brief The VertexAttrib class
 *
 * The attributes of a single vertex, used to build PolygonMesh objects: a position and a
 * normal, both stored as homogeneous 4-vectors.
 *
 * @author Yu Cao
 * @date 2026-03-18
 */
#ifndef VERTEXATTRIB_H
#define VERTEXATTRIB_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "util/IVertexData.h"

class VertexAttrib : public util::IVertexData
{
public:
    VertexAttrib() : position(0, 0, 0, 1), normal(0, 0, 0, 0) {}

    /**
     * @brief Check if the vertex has the attribute
     *
     * @param[in]   attribName  The name of the attribute
     * @return true for "position" and "normal"
     */
    bool hasData(const std::string &attribName) const override
    {
        return attribName == "position" || attribName == "normal";
    }

    /**
     * @brief Get the data of the attribute
     *
     * @param[in]   attribName  The name of the attribute
     * @return The four homogeneous coordinates, empty for an unknown attribute
     */
    std::vector<float> getData(const std::string &attribName) const override
    {
        if (attribName == "position")
        {
            return {position.x, position.y, position.z, position.w};
        }
        if (attribName == "normal")
        {
            return {normal.x, normal.y, normal.z, normal.w};
        }
        return {};
    }

    /**
     * @brief Set the data of the attribute; unknown attributes are ignored
     *
     * @param[in]   attribName  The name of the attribute
     * @param[in]   data        One to four coordinates; missing ones keep their defaults,
     *                          (0, 0, 0, 1) for a position and (0, 0, 0, 0) for a normal
     */
    void setData(const std::string &attribName, const std::vector<float> &data) override
    {
        glm::vec4 *target = nullptr;
        if (attribName == "position")
        {
            position = glm::vec4(0, 0, 0, 1);
            target = &position;
        }
        else if (attribName == "normal")
        {
            normal = glm::vec4(0, 0, 0, 0);
            target = &normal;
        }
        else
        {
            return;
        }

        for (size_t i = 0; i < data.size() && i < 4; i++)
        {
            (*target)[static_cast<glm::length_t>(i)] = data[i];
        }
    }

    /**
     * @brief Get the names of all attributes this class supports
     *
     * @return {"position", "normal"}
     */
    std::vector<std::string> getAllAttributes() const override
    {
        return {"position", "normal"};
    }

private:
    glm::vec4 position;
    glm::vec4 normal;
};

#endif
