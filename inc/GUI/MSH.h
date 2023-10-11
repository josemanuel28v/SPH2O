#pragma once
#include "Mesh3D.h"
#include <memory>
#include <vector>

class MSH
{
public:
	const std::vector<Mesh3D::ptr>& getMeshes() { return meshes; }
	void addMesh(Mesh3D::ptr mesh) { meshes.push_back(mesh); }
private:
	std::vector<Mesh3D::ptr> meshes;
};

