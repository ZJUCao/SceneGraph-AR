/**
 * @file ScenegraphImporter.h
 *
 * @brief The ScenegraphImporter class
 *
 * Builds a scene graph from the text scene description: meshes, materials, group, leaf and
 * transform nodes, subtree copies and imports of other scene files. Everything after a '#' on
 * a line is a comment.
 *
 * @author Amit Shesh
 */
#ifndef SGRAPH_SCENEGRAPHIMPORTER_H
#define SGRAPH_SCENEGRAPHIMPORTER_H

#include <glm/glm.hpp>
#include <fstream>
#include <istream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include "VertexAttrib.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/RotateTransform.h"
#include "sgraph/ScaleTransform.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/TranslateTransform.h"
#include "util/Material.h"
#include "util/ObjImporter.h"
#include "util/PolygonMesh.h"

namespace sgraph
{
    /**
     * @brief Import a scene graph from an input stream.
     */
    class ScenegraphImporter
    {
    public:
        ScenegraphImporter() : root(nullptr) {}

        virtual ~ScenegraphImporter() {}

        /**
         * @brief Parse the input stream and return the scene graph
         *
         * Mesh and imported scene paths in the stream are relative to the working directory.
         *
         * @param input The input stream to parse
         * @return The scene graph parsed from the input stream; the caller owns it
         * @throws std::runtime_error on an unknown command, a missing file, or a missing root
         */
        IScenegraph *parse(std::istream &input)
        {
            std::istringstream inputWithoutComments(stripComments(input));
            std::string command;
            while (inputWithoutComments >> command)
            {
                if (command == "instance")
                {
                    parseInstance(inputWithoutComments);
                }
                else if (command == "group")
                {
                    parseGroup(inputWithoutComments);
                }
                else if (command == "leaf")
                {
                    parseLeaf(inputWithoutComments);
                }
                else if (command == "material")
                {
                    parseMaterial(inputWithoutComments);
                }
                else if (command == "scale")
                {
                    parseScale(inputWithoutComments);
                }
                else if (command == "rotate")
                {
                    parseRotate(inputWithoutComments);
                }
                else if (command == "translate")
                {
                    parseTranslate(inputWithoutComments);
                }
                else if (command == "copy")
                {
                    parseCopy(inputWithoutComments);
                }
                else if (command == "import")
                {
                    parseImport(inputWithoutComments);
                }
                else if (command == "assign-material")
                {
                    parseAssignMaterial(inputWithoutComments);
                }
                else if (command == "add-child")
                {
                    parseAddChild(inputWithoutComments);
                }
                else if (command == "assign-root")
                {
                    parseSetRoot(inputWithoutComments);
                }
                else
                {
                    throw std::runtime_error("Unrecognized or out-of-place command: " + command);
                }
            }

            if (root == nullptr)
            {
                throw std::runtime_error("Parsed scene graph, but nothing set as root");
            }

            IScenegraph *scenegraph = new Scenegraph();
            scenegraph->makeScenegraph(root);
            scenegraph->setMeshes(meshes);
            scenegraph->setMeshPaths(meshPaths);
            return scenegraph;
        }

    protected:
        virtual void parseInstance(std::istream &input)
        {
            std::string name, path;
            input >> name >> path;

            std::ifstream in(path);
            if (!in.is_open())
            {
                throw std::runtime_error("Cannot open mesh file: " + path);
            }
            meshPaths[name] = path;
            meshes[name] = util::ObjImporter<VertexAttrib>::importFile(in, false);
        }

        virtual void parseGroup(std::istream &input)
        {
            std::string varname, name;
            input >> varname >> name;
            nodes[varname] = new GroupNode(name, nullptr);
        }

        virtual void parseLeaf(std::istream &input)
        {
            std::string varname, name, command, instanceOf;
            input >> varname >> name >> command;
            if (command == "instanceof")
            {
                input >> instanceOf;
            }
            nodes[varname] = new LeafNode(instanceOf, name, nullptr);
        }

        virtual void parseScale(std::istream &input)
        {
            std::string varname, name;
            float sx, sy, sz;
            input >> varname >> name >> sx >> sy >> sz;
            nodes[varname] = new ScaleTransform(sx, sy, sz, name, nullptr);
        }

        virtual void parseTranslate(std::istream &input)
        {
            std::string varname, name;
            float tx, ty, tz;
            input >> varname >> name >> tx >> ty >> tz;
            nodes[varname] = new TranslateTransform(tx, ty, tz, name, nullptr);
        }

        virtual void parseRotate(std::istream &input)
        {
            std::string varname, name;
            float angleInDegrees, ax, ay, az;
            input >> varname >> name >> angleInDegrees >> ax >> ay >> az;
            nodes[varname] =
                new RotateTransform(glm::radians(angleInDegrees), ax, ay, az, name, nullptr);
        }

        virtual void parseMaterial(std::istream &input)
        {
            util::Material mat;
            float r, g, b;
            std::string name, command;
            input >> name >> command;
            while (input && command != "end-material")
            {
                if (command == "ambient")
                {
                    input >> r >> g >> b;
                    mat.setAmbient(r, g, b);
                }
                else if (command == "diffuse")
                {
                    input >> r >> g >> b;
                    mat.setDiffuse(r, g, b);
                }
                else if (command == "specular")
                {
                    input >> r >> g >> b;
                    mat.setSpecular(r, g, b);
                }
                else if (command == "emission")
                {
                    input >> r >> g >> b;
                    mat.setEmission(r, g, b);
                }
                else if (command == "shininess")
                {
                    input >> r;
                    mat.setShininess(r);
                }
                input >> command;
            }
            materials[name] = mat;
        }

        virtual void parseCopy(std::istream &input)
        {
            std::string nodename, copyof;
            input >> nodename >> copyof;
            SGNode *original = findNode(copyof);
            if (original != nullptr)
            {
                nodes[nodename] = original->clone();
            }
        }

        virtual void parseImport(std::istream &input)
        {
            std::string nodename, filepath;
            input >> nodename >> filepath;

            std::ifstream external(filepath);
            if (!external.is_open())
            {
                throw std::runtime_error("Cannot open imported scene file: " + filepath);
            }

            // The imported file shares this importer's nodes, materials and meshes, so its names
            // can be used afterwards. It must set its own root, which becomes the new node.
            SGNode *outerRoot = root;
            root = nullptr;
            IScenegraph *imported = parse(external);
            nodes[nodename] = imported->getRoot();
            imported->makeScenegraph(nullptr); // keep the subtree alive after deleting the graph
            delete imported;
            root = outerRoot;
        }

        virtual void parseAssignMaterial(std::istream &input)
        {
            std::string nodename, matname;
            input >> nodename >> matname;

            LeafNode *leafNode = dynamic_cast<LeafNode *>(findNode(nodename));
            auto material = materials.find(matname);
            if ((leafNode != nullptr) && (material != materials.end()))
            {
                leafNode->setMaterial(material->second);
            }
        }

        virtual void parseAddChild(std::istream &input)
        {
            std::string childname, parentname;
            input >> childname >> parentname;

            ParentSGNode *parentNode = dynamic_cast<ParentSGNode *>(findNode(parentname));
            SGNode *childNode = findNode(childname);
            if ((parentNode != nullptr) && (childNode != nullptr))
            {
                parentNode->addChild(childNode);
            }
        }

        virtual void parseSetRoot(std::istream &input)
        {
            std::string rootname;
            input >> rootname;
            root = findNode(rootname);
        }

        /**
         * @brief Look up a node by its variable name in the scene file
         *
         * @return the node, or null if no node has that name
         */
        SGNode *findNode(const std::string &varname) const
        {
            auto it = nodes.find(varname);
            return (it != nodes.end()) ? it->second : nullptr;
        }

        /**
         * @brief Remove everything from a '#' to the end of each line
         */
        static std::string stripComments(std::istream &input)
        {
            std::string line;
            std::ostringstream clean;
            while (std::getline(input, line))
            {
                clean << line.substr(0, line.find('#')) << '\n';
            }
            return clean.str();
        }

    private:
        std::map<std::string, SGNode *> nodes;
        std::map<std::string, util::Material> materials;
        std::map<std::string, util::PolygonMesh<VertexAttrib>> meshes;
        std::map<std::string, std::string> meshPaths;
        SGNode *root;
    };
} // namespace sgraph

#endif
