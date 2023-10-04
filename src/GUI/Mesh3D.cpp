#include "Mesh3D.h"

uint Mesh3D::counter = 0;

Mesh3D::Mesh3D()
{
	id = counter++;

	vertices = new std::vector<vertex_t>();
	indices = new std::vector<glm::uint32>();
	material = nullptr;
}

Mesh3D::~Mesh3D()
{
	if (material) delete material;

	vertices->clear();
	indices->clear();

	delete vertices;
	delete indices;
}

uint Mesh3D::getMeshID()
{
	return id;
}

void Mesh3D::addVertex(vertex_t v)
{
	vertices->push_back(v);
}

std::vector<struct vertex_t>* Mesh3D::getVertices()
{
	return vertices;
}

Material* Mesh3D::getMaterial()
{
	return material;
}

void Mesh3D::setMaterial(Material* material)
{
	this->material = material;
}

void Mesh3D::setVertices(std::vector<struct vertex_t>* vertices)
{
	this->vertices = vertices;
}

void Mesh3D::setIndices(std::vector<glm::uint32>* indices)
{
	this->indices = indices;
}

std::vector<glm::uint32>* Mesh3D::getIndices()
{
	return indices;
}

void Mesh3D::addTriangle(glm::uint32 id1, glm::uint32 id2, glm::uint32 id3)
{
	indices->push_back(id1);
	indices->push_back(id2);
	indices->push_back(id3);
}
