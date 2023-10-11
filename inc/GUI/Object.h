#pragma once
#include "Entity.h"
#include "Mesh3D.h"
#include "State.h"
#include "MSH.h"
#include <string>
#include <memory>

class Object: public Entity
{
	std::vector<Mesh3D::ptr> meshes;
	uint type;

public:
	Object() {}
	Object(std::string fileName);
	virtual void load(std::string);
	virtual void setMesh(Mesh3D::ptr mesh);
	virtual std::vector<Mesh3D::ptr>& getMeshes();
};

