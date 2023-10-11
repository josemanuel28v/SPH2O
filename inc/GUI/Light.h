#pragma once
#include "Entity.h"

class Light: public Entity
{

public:

	enum Type {
		DIRECTIONAL = 0, POINT = 1
	};

	struct LightInfo {
		glm::vec3 color;
		Type type;
		float intensity;
		float linearAtt;
	};

	Light() {}
	Light(glm::vec3 position, glm::vec3 color, Type type, float intensity, float linearAtt);
	Type getType() const;
	void setType(Type type);
	const glm::vec3& getColor() const;
	void setColor(const glm::vec3& color);
	virtual void step(double timeStep);
	float getLinearAttenuation() const;
	void setLinearAttenuation(float att);
	const LightInfo& getInfo() const;
	void setInfo(LightInfo info);

protected:

	LightInfo info;
};

