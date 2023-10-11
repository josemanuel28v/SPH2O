#include "GUI/OrbitalCamera.h"

void OrbitalCamera::step(double deltaTime)
{
	static float angle = 0;

	position.x = lookAt.x + radius * glm::cos(angle);
	position.z = lookAt.z + radius * glm::sin(angle);

	angle += static_cast<float>(deltaTime);

	computeViewMatrix();
}
