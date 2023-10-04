#include "OrbitalLight.h"

OrbitalLight::OrbitalLight(glm::vec3 center, glm::vec3 color, float linearAtt, float radius)
{
	this->info.color = color;
	this->info.type = POINT;
	this->info.linearAtt = linearAtt;
	this->center = center;
	this->radius = radius;
	this->position = glm::vec4(center, 1.0);
}

void OrbitalLight::step(double deltaTime)
{
	static float angle = 0;

	position.x = center.x + radius * cos(angle);
	position.z = center.z + radius * sin(angle);

	angle += static_cast<float>(deltaTime);
}
