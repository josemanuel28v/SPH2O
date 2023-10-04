#pragma once
#include "Object3d.h"

class Cube : public Object3D
{
public:
	Cube();
	void step(double deltaTime) override;
};

