#pragma once
#include "Object3d.h"

class CubeTex : public Object3D
{
public:
	CubeTex();
	void step(double deltaTime) override;
};

