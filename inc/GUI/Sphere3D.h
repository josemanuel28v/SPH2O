#pragma once
#include "Object3d.h"

class Sphere3D : public Object3D
{
public:
	Sphere3D();
	void step(double deltaTime) override;
};

