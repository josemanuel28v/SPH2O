#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 color, Type type, float intensity, float linearAtt)
{
    info = { color, type, intensity, linearAtt };
    this->position = glm::vec4(position, 1.0f);
}

Light::Type Light::getType() const
{
    return info.type;
}

void Light::setType(Type type)
{
    this->info.type = type;
}

const glm::vec3& Light::getColor() const
{
    return info.color;
}

void Light::setColor(const glm::vec3& color)
{
    this->info.color = color;
}

void Light::step(double timeStep)
{
}

float Light::getLinearAttenuation() const
{
    return info.linearAtt;
}

void Light::setLinearAttenuation(float att)
{
    this->info.linearAtt = att;
}

const Light::LightInfo& Light::getInfo() const
{
    return info;
}

void Light::setInfo(LightInfo info)
{
    this->info = info;
}
