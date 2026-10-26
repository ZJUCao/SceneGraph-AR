/**
 * @file Material.h
 *
 * @brief The Material class
 *
 * A material is represented by its ambient, diffuse, specular and emission colors, plus
 * coefficients for shininess, absorption, reflection, transparency and refractive index. The
 * AR overlay fills each mesh with its ambient color.
 *
 * @author Amit Shesh
 */
#ifndef UTIL_MATERIAL_H
#define UTIL_MATERIAL_H

#include <glm/glm.hpp>

namespace util
{
    /**
     * This class represents material. Material is represented using (a) ambient (b) diffuse
     * (c) specular (d) emission (for materials that emit light themselves).
     * It also has coefficients for shininess, absorption, reflection (for reflective
     * material), transparency (for transparent material) and refractive index (for
     * transparent material). For the latter ones, the user must make sure that
     * absorption + reflection + transparency = 1
     */
    class Material
    {
    public:
        Material()
        {
            init();
        }

        glm::vec4 getEmission() const
        {
            return emission;
        }

        void setEmission(float r, float g, float b)
        {
            emission = glm::vec4(r, g, b, 1);
        }

        void setEmission(const glm::vec4 &v)
        {
            emission = v;
        }

        glm::vec4 getAmbient() const
        {
            return ambient;
        }

        void setAmbient(float r, float g, float b)
        {
            ambient = glm::vec4(r, g, b, 1);
        }

        void setAmbient(const glm::vec4 &v)
        {
            ambient = v;
        }

        glm::vec4 getDiffuse() const
        {
            return diffuse;
        }

        void setDiffuse(float r, float g, float b)
        {
            diffuse = glm::vec4(r, g, b, 1);
        }

        void setDiffuse(const glm::vec4 &v)
        {
            diffuse = v;
        }

        glm::vec4 getSpecular() const
        {
            return specular;
        }

        void setSpecular(float r, float g, float b)
        {
            specular = glm::vec4(r, g, b, 1);
        }

        void setSpecular(const glm::vec4 &v)
        {
            specular = v;
        }

        float getShininess() const
        {
            return shininess;
        }

        void setShininess(float r)
        {
            shininess = r;
        }

        float getAbsorption() const
        {
            return absorption;
        }

        void setAbsorption(float a)
        {
            absorption = a;
        }

        float getReflection() const
        {
            return reflection;
        }

        void setReflection(float r)
        {
            reflection = r;
        }

        float getTransparency() const
        {
            return transparency;
        }

        void setTransparency(float t)
        {
            transparency = t;
            ambient.w = diffuse.w = specular.w = emission.w = 1 - t;
        }

        float getRefractiveIndex() const
        {
            return refractiveIndex;
        }

        void setRefractiveIndex(float r)
        {
            refractiveIndex = r;
        }

    private:
        void init()
        {
            setEmission(0.0f, 0.0f, 0.0f);
            setAmbient(0.0f, 0.0f, 0.0f);
            setDiffuse(0.0f, 0.0f, 0.0f);
            setSpecular(0.0f, 0.0f, 0.0f);
            setShininess(0.0f);
            setAbsorption(1);
            setReflection(0);
            setTransparency(0);
            setRefractiveIndex(1);
        }

        glm::vec4 emission;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        float shininess;
        float absorption;
        float reflection;
        float transparency;
        float refractiveIndex;
    };
} // namespace util

#endif
