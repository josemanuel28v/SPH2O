#pragma once
#include "Object3d.h"

class ObjModel : public Object3D
{
public:
	ObjModel(const std::string& path);
	void step(double deltaTime) override;
};

