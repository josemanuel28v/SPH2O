#include "CustomParticle.h"
#include "GLTexture.h"
#include "GLSLMaterial.h"

CustomParticle::CustomParticle(float radius) : Particle(radius)
{
    // Load texture
    Texture* texture = new GLTexture("../../data/textures/particle.png");

    // Set program and texture to material
    Material* material = new GLSLMaterial();
    material->loadPrograms({"../../data/shaders/particle.vert", "../../data/shaders/particle.frag"});
    material->setTexture(texture);
    material->setTexturing(true);
    material->setLighting(true);
    material->setDepthWrite(true);
    material->setCulling(false);
    material->setBlendMode(Material::SOLID);
    material->setReflection(false);
    material->setRefraction(false);
    material->setNormalMapping(false);
    material->setShininess(10);

    // Set material to mesh
    Mesh3D* mesh = new Mesh3D();
    mesh->setMaterial(material);

    std::vector<vertex_t> vertices(4);
    std::vector<glm::uint32> indices(6);

    vertices[0].position = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
    vertices[1].position = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
    vertices[2].position = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
    vertices[3].position = glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);

    vertices[0].textCoord = glm::vec2(0.0f, 0.0f);
    vertices[1].textCoord = glm::vec2(0.0f, 1.0f);
    vertices[2].textCoord = glm::vec2(1.0f, 1.0f);
    vertices[3].textCoord = glm::vec2(1.0f, 0.0f);

    vertices[0].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vertices[1].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vertices[2].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vertices[3].normal = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;

    *(mesh->getVertices()) = vertices;
    *(mesh->getIndices()) = indices;

    // Set mesh to particle (Cuidado no se esta añadiendo la mesh al State)
    Mesh3D::ptr sharedMesh(mesh);
    setMesh(sharedMesh);
}