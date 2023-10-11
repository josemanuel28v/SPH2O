#include "GUI/Object.h"
#include "pugixml.hpp"
#include "GUI/FactoryEngine.h"
#include "GUI/utils.h"
#include <iostream>

Object::Object(std::string fileName)
{
    load(fileName);
}

void Object::load(std::string fileName)
{
    // Comprobar si ya se ha cargado el fichero .msh anteriormente
    if (State::existMSH(fileName))
    {
        MSH* meshes = State::getMSH(fileName);
        for (Mesh3D::ptr mesh : meshes->getMeshes())
        {
            this->setMesh(mesh);
        }

        return;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(fileName.c_str());

    if (result)
    {
        MSH* msh = new MSH();
        std::string path = utils::extractPath(fileName);
        pugi::xml_node buffersNode = doc.child("mesh").child("buffers");

        for (pugi::xml_node bufferNode = buffersNode.child("buffer");
            bufferNode;
            bufferNode = bufferNode.next_sibling("buffer"))
        {
            Mesh3D* mesh = new Mesh3D();
            Material* mat = FactoryEngine::getNewMaterial();
            mesh->setMaterial(mat);

            // Color
            if (bufferNode.child("material").child("color"))
            {
                std::vector<float> listcolor = utils::splitString<float>(bufferNode.child("material").child("color").text().as_string(), ',');
                mat->setColor(glm::vec4(listcolor[0], listcolor[1], listcolor[2], 1.0f));
            }

            // Shininess
            if (bufferNode.child("material").child("shininess"))
            {
                mat->setShininess(bufferNode.child("material").child("shininess").text().as_int());
            }

            // Reflection
            if (bufferNode.child("material").child("reflection"))
            {
                mat->setReflection(bufferNode.child("material").child("reflection").text().as_bool());

                if (mat->getReflection())
                {
                    std::vector<std::string> cubeMapFiles = utils::splitString<std::string>(bufferNode.child("material").child("reflectionTexture").text().as_string(), ',');
                    Texture* texture = FactoryEngine::getNewTexture();
                    texture->load(path + cubeMapFiles[0], path + cubeMapFiles[1], path + cubeMapFiles[2], path + cubeMapFiles[3], path + cubeMapFiles[4], path + cubeMapFiles[5]);

                    mat->setReflectionMap(texture);
                }
            }

            // Refraction
            if (bufferNode.child("material").child("refraction"))
            {
                mat->setRefraction(bufferNode.child("material").child("refraction").text().as_bool());

                if (mat->getRefraction())
                {
                    std::vector<std::string> cubeMapFiles = utils::splitString<std::string>(bufferNode.child("material").child("refractionTexture").text().as_string(), ',');
                    Texture* texture = FactoryEngine::getNewTexture();
                    texture->load(path + cubeMapFiles[0], path + cubeMapFiles[1], path + cubeMapFiles[2], path + cubeMapFiles[3], path + cubeMapFiles[4], path + cubeMapFiles[5]);

                    mat->setRefractionMap(texture);
                    mat->setRefractCoef(bufferNode.child("material").child("refractCoef").text().as_float());
                }
            }

            // Texturas
            if (bufferNode.child("material").child("texture"))
            {
                // Skybox (COLOR3D)
                auto cubeMapNode = bufferNode.child("material").child("texture").attribute("cubeMap");
                if (cubeMapNode && cubeMapNode.as_bool())
                {
                    std::vector<std::string> cubeMapFiles = utils::splitString<std::string>(bufferNode.child("material").child("texture").text().as_string(), ',');
                    Texture* texture = FactoryEngine::getNewTexture();
                    texture->load(path + cubeMapFiles[0], path + cubeMapFiles[1], path + cubeMapFiles[2], path + cubeMapFiles[3], path + cubeMapFiles[4], path + cubeMapFiles[5]);

                    mat->setTexture(texture);
                    mat->setTexturing(true);
                }
                // colorTexture (COLOR2D)
                else
                {
                    std::string textureFile = path + bufferNode.child("material").child("texture").text().as_string();

                    Texture* texture = FactoryEngine::getNewTexture();
                    texture->load(textureFile);
                    mat->setTexture(texture);
                    mat->setTexturing(true);
                }
            }

            // Normal map 
            if (bufferNode.child("material").child("normalTexture"))
            {
                std::string textureFile = path + bufferNode.child("material").child("normalTexture").text().as_string();

                Texture* texture = FactoryEngine::getNewTexture();
                texture->load(textureFile);
                mat->setNormalMap(texture);
                mat->setNormalMapping(true);
            }

            // Shaders
            if (bufferNode.child("material").child("vShader") && bufferNode.child("material").child("fShader"))
            {
                std::string vShader = path + bufferNode.child("material").child("vShader").text().as_string();
                std::string fShader = path + bufferNode.child("material").child("fShader").text().as_string();
                mat->loadPrograms({ vShader, fShader });
            }
            else
            {
                std::string vShader = "data/default.vertex";
                std::string fShader = "data/default.fragment";
                mat->loadPrograms({ vShader, fShader });
            }

            // Iluminaci�n 
            if (bufferNode.child("material").child("light"))
            {
                mat->setLighting(bufferNode.child("material").child("light").text().as_bool());
            }

            // Depth buffer
            if (bufferNode.child("material").child("depthWrite"))
            {
                mat->setDepthWrite(bufferNode.child("material").child("depthWrite").text().as_bool());
            }

            // Backface culling
            if (bufferNode.child("material").child("culling"))
            {
                mat->setCulling(bufferNode.child("material").child("culling").text().as_bool());
            }

            // Blend mode
            if (bufferNode.child("material").child("blendMode"))
            {
                std::string blendMode = bufferNode.child("material").child("blendMode").text().as_string();

                if (blendMode == "alpha")
                {
                    mat->setBlendMode(Material::ALPHA);
                }
                //else if (blendMode == "mul")...
            }
            else
            {
                mat->setBlendMode(Material::SOLID);
            }

            std::vector<float> vList = utils::splitString<float>(bufferNode.child("coords").text().as_string(), ',');
            std::vector<float> tcList;
            std::vector<float> nList;
            std::vector<float> tanList;

            // Coordenadas de textura
            if (mat->getTexturing())
            {
                tcList = utils::splitString<float>(bufferNode.child("texCoords").text().as_string(), ',');
            }

            // Normales
            if (bufferNode.child("normals"))
            {
                nList = utils::splitString<float>(bufferNode.child("normals").text().as_string(), ',');
            }

            // Tangentes
            if (bufferNode.child("tangents"))
            {
                tanList = utils::splitString<float>(bufferNode.child("tangents").text().as_string(), ',');
            }

            auto coord = vList.begin();
            auto texCoord = tcList.begin();
            auto normals = nList.begin();
            auto tans = tanList.begin();

            while (coord != vList.end())
            {
                vertex_t v;

                v.position.x = *coord++;
                v.position.y = *coord++;
                v.position.z = *coord++;
                v.position.w = 1.0f;

                if (tcList.size() > 0)
                {
                    v.textCoord.x = *texCoord++;
                    v.textCoord.y = *texCoord++;
                }

                if (nList.size() > 0)
                {
                    v.normal.x = *normals++;
                    v.normal.y = *normals++;
                    v.normal.z = *normals++;
                    v.normal.w = 0.0f;
                }

                if (tanList.size() > 0)
                {
                    v.tangent.x = *tans++;
                    v.tangent.y = *tans++;
                    v.tangent.z = *tans++;
                    v.tangent.w = 0.0f;
                }

                mesh->addVertex(v);
            }

            *(mesh->getIndices()) = utils::splitString<uint>(bufferNode.child("indices").text().as_string(), ',');

            Mesh3D::ptr sharedMesh(mesh);
            this->setMesh(sharedMesh);
            msh->addMesh(sharedMesh);
        }

        State::addMSH(fileName, msh);
    }
    else
    {
        std::cout << "No se ha podido cargar la imagen " << fileName << std::endl;
        std::cout << result.description() << std::endl;
    }
}

void Object::setMesh(Mesh3D::ptr mesh)
{
	meshes.push_back(mesh);
}

std::vector<Mesh3D::ptr>& Object::getMeshes()
{
	return meshes;
}

