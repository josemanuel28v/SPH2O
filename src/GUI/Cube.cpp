#include "Cube.h"
#include "GLTexture.h"
#include "FactoryEngine.h"

Cube::Cube()
{
	Mesh3D* verticalFaces = new Mesh3D();
	Mesh3D* horizontalFaces = new Mesh3D();

	// Cara vertical frontal
	vertex_t v0, v1, v2, v3;
	
	v0.position = glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f);
	v1.position = glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f);
	v2.position = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // 1    2
	v3.position = glm::vec4(0.5f, -0.5f, 0.5f, 1.0f); // 0    3

	v0.textCoord = glm::vec2(0.0f, 0.0f);
	v1.textCoord = glm::vec2(0.0f, 1.0f);
	v2.textCoord = glm::vec2(1.0f, 1.0f);
	v3.textCoord = glm::vec2(1.0f, 0.0f);

	v0.normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0);
	v1.normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0);
	v2.normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0);
	v3.normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0);

	verticalFaces->addVertex(v0);
	verticalFaces->addVertex(v1);
	verticalFaces->addVertex(v2);
	verticalFaces->addVertex(v3);

	verticalFaces->addTriangle(1, 0, 2);
	verticalFaces->addTriangle(3, 2, 0);

	// Cara vertical izquierda (izquierda de frente)
	vertex_t v4, v5, v6, v7;

	v4.position = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
	v5.position = glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f);
	v6.position = v1.position;
	v7.position = v0.position;

	v4.textCoord = glm::vec2(0.0f, 0.0f);
	v5.textCoord = glm::vec2(0.0f, 1.0f);
	v6.textCoord = glm::vec2(1.0f, 1.0f);
	v7.textCoord = glm::vec2(1.0f, 0.0f);

	v4.normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0);
	v5.normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0);
	v6.normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0);
	v7.normal = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0);

	verticalFaces->addVertex(v4);
	verticalFaces->addVertex(v5);
	verticalFaces->addVertex(v6);
	verticalFaces->addVertex(v7);

	verticalFaces->addTriangle(5, 4, 6);
	verticalFaces->addTriangle(7, 6, 4);

	// Cara derecha (derecha de frente)
	vertex_t v8, v9, v10, v11;

	v8.position = v3.position;
	v9.position = v2.position;
	v10.position = glm::vec4(0.5f, 0.5f, -0.5f, 1.0f); // 1    2
	v11.position = glm::vec4(0.5f, -0.5f, -0.5f, 1.0f); // 0    3

	v8.textCoord = glm::vec2(0.0f, 0.0f);
	v9.textCoord = glm::vec2(0.0f, 1.0f);
	v10.textCoord = glm::vec2(1.0f, 1.0f);
	v11.textCoord = glm::vec2(1.0f, 0.0f);

	v8.normal = glm::vec4(1.0f, 0.0f, 0.0f, 1.0);
	v9.normal = glm::vec4(1.0f, 0.0f, 0.0f, 1.0);
	v10.normal = glm::vec4(1.0f, 0.0f, 0.0f, 1.0);
	v11.normal = glm::vec4(1.0f, 0.0f, 0.0f, 1.0);

	verticalFaces->addVertex(v8);
	verticalFaces->addVertex(v9);
	verticalFaces->addVertex(v10);
	verticalFaces->addVertex(v11);

	verticalFaces->addTriangle(9, 8, 10);
	verticalFaces->addTriangle(11, 10, 8);

	// Cara trasera
	vertex_t v12, v13, v14, v15;

	v12.position = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
	v13.position = glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f);
	v14.position = glm::vec4(0.5f, 0.5f, -0.5f, 1.0f); // 1    2
	v15.position = glm::vec4(0.5f, -0.5f, -0.5f, 1.0f); // 0    3

	v12.textCoord = glm::vec2(0.0f, 0.0f);
	v13.textCoord = glm::vec2(0.0f, 1.0f);
	v14.textCoord = glm::vec2(1.0f, 1.0f);
	v15.textCoord = glm::vec2(1.0f, 0.0f);

	v12.normal = glm::vec4(0.0f, 0.0f, -1.0f, 1.0);
	v13.normal = glm::vec4(0.0f, 0.0f, -1.0f, 1.0);
	v14.normal = glm::vec4(0.0f, 0.0f, -1.0f, 1.0);
	v15.normal = glm::vec4(0.0f, 0.0f, -1.0f, 1.0);

	verticalFaces->addVertex(v12);
	verticalFaces->addVertex(v13);
	verticalFaces->addVertex(v14);
	verticalFaces->addVertex(v15);

	verticalFaces->addTriangle(12, 13, 14);
	verticalFaces->addTriangle(14, 15, 12);

	// Cara superior
	vertex_t v16, v17, v18, v19;

	v16.position = v1.position;
	v17.position = v5.position;
	v18.position = v10.position;
	v19.position = v2.position;

	v16.textCoord = glm::vec2(0.0f, 0.0f);
	v17.textCoord = glm::vec2(0.0f, 1.0f);
	v18.textCoord = glm::vec2(1.0f, 1.0f);
	v19.textCoord = glm::vec2(1.0f, 0.0f);

	v16.normal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0);
	v17.normal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0);
	v18.normal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0);
	v19.normal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0);

	horizontalFaces->addVertex(v16);
	horizontalFaces->addVertex(v17);
	horizontalFaces->addVertex(v18);
	horizontalFaces->addVertex(v19);

	horizontalFaces->addTriangle(0, 2, 1);
	horizontalFaces->addTriangle(3, 2, 0);

	// Cara inferior
	vertex_t v20, v21, v22, v23;

	v20.position = v4.position; 
	v21.position = v0.position; 
	v22.position = v3.position;
	v23.position = v11.position; 

	v20.textCoord = glm::vec2(0.0f, 0.0f);
	v21.textCoord = glm::vec2(0.0f, 1.0f);
	v22.textCoord = glm::vec2(1.0f, 1.0f);
	v23.textCoord = glm::vec2(1.0f, 0.0f);

	v20.normal = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	v21.normal = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	v22.normal = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	v23.normal = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);

	horizontalFaces->addVertex(v20);
	horizontalFaces->addVertex(v21);
	horizontalFaces->addVertex(v22);
	horizontalFaces->addVertex(v23);

	horizontalFaces->addTriangle(5, 4, 6);
	horizontalFaces->addTriangle(7, 6, 4);

	// MATERIALES

	Material* frontMat = FactoryEngine::getNewMaterial();
	Material* topMat = FactoryEngine::getNewMaterial();

	frontMat->loadPrograms({"../../data/shaders/program.vert", "../../data/shaders/program.frag"});
	frontMat->setShininess(50);
	frontMat->setTexturing(false);
	frontMat->setLighting(true);
	frontMat->setDepthWrite(true);
	frontMat->setCulling(false);
	frontMat->setWireframe(true);

	topMat->loadPrograms({ "../../data/shaders/program.vert", "../../data/shaders/program.frag" });
	topMat->setShininess(50);
	topMat->setTexturing(false);
	topMat->setLighting(false);
	topMat->setDepthWrite(true);
	topMat->setCulling(false);
	topMat->setWireframe(true);

	verticalFaces->setMaterial(frontMat);
	horizontalFaces->setMaterial(topMat);

	this->setMesh(Mesh3D::ptr(verticalFaces));
	this->setMesh(Mesh3D::ptr(horizontalFaces));
}

void Cube::step(double deltaTime)
{}

