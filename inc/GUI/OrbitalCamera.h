#pragma once

#include "Camera.h"

class OrbitalCamera : public Camera
{
public:
	OrbitalCamera(ProjectionType type, float height, glm::vec3 up, glm::vec3 lookAt, float radius) : Camera(type, glm::vec3(0.0f, height, 0.0f), up, lookAt) 
	{ 
		this->radius = radius; this->up = up; 
	}
	virtual void step(double deltaTime) override;
private:
	float radius;
};

