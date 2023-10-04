#pragma once
#include "Object.h"


class Object3D : public Object
{
public:
	
	Object3D() {}
	Object3D(std::string fileName) : Object(fileName) {}
	virtual void step(double deltaTime) override {}
};

