#include "GUI/TrianguloRot.h"
#include "GUI/FactoryEngine.h"

TrianguloRot::TrianguloRot()
{
    Mesh3D* triangle_mesh = new Mesh3D();
    vertex_t v1, v2, v3;

    v1.position = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
    v2.position = glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
    v3.position = glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);

    triangle_mesh->addVertex(v1);
    triangle_mesh->addVertex(v2);
    triangle_mesh->addVertex(v3);
    triangle_mesh->addTriangle(0, 1, 2);

    Material* mat = FactoryEngine::getNewMaterial();
    triangle_mesh->setMaterial(mat);

    mat->loadPrograms({"data/default.vertex", "data/default.fragment"});

    setMesh(Mesh3D::ptr(triangle_mesh));
}

void TrianguloRot::step(double deltaTime)
{
    float rotationVelocity = 1;
    rotation.y += rotationVelocity * static_cast<float>(deltaTime);
}
