#pragma once
#include "Light.h"

class OrbitalLight : public Light
{
public:

	OrbitalLight(glm::vec3 center, glm::vec3 color, float linearAtt, float radius);
	virtual void step(double deltaTime) override;
private:
	float radius = 1;
	glm::vec3 center;

};

