#include "Material.h"

Material::~Material()
{
    if (colorMap) delete colorMap;
    if (normalMap) delete normalMap;
    if (program) delete program;

    colorMap = nullptr;
    normalMap = nullptr;
    program = nullptr;
}

void Material::setTexturing(bool texturing)
{
    this->texturing = texturing;
}

void Material::setLighting(bool lighting)
{
    this->lighting = lighting;
}

void Material::setCulling(bool culling)
{
    this->culling = culling;
}

void Material::setDepthWrite(bool depthWrite)
{
    this->depthWrite = depthWrite;
}

void Material::setBlendMode(BlendMode blendMode)
{
    this->blendMode = blendMode;
}

void Material::setNormalMapping(bool normalMapping)
{
    this->normalMapping = normalMapping;
}

void Material::setTexture(Texture* colorMap)
{
    this->colorMap = colorMap;
}

void Material::setRefractionMap(Texture* refractionMap)
{
    this->refractionMap = refractionMap;
}

void Material::setReflectionMap(Texture* reflectionMap)
{
    this->reflectionMap = reflectionMap;
}

void Material::setNormalMap(Texture* normalMap)
{
    this->normalMap = normalMap;
}

void Material::setProgram(RenderProgram* program)
{
    this->program = program;
}

void Material::setColor(glm::vec4 color)
{
    this->color = color;
}

void Material::setShininess(uint8_t shininess)
{
    this->shininess = shininess;
}

void Material::setRefraction(bool refraction)
{
    this->refraction = refraction;
}

void Material::setReflection(bool reflection)
{
    this->reflection = reflection;
}

void Material::setRefractCoef(float refractCoef)
{
    this->refractCoef = refractCoef;
}

void Material::setWireframe(bool wireframe)
{
    this->wireframe = wireframe;
}

bool Material::getTexturing() const
{
    return texturing;
}

bool Material::getLighting() const
{
    return lighting;
}

bool Material::getCulling() const
{
    return culling;
}

bool Material::getDepthWrite() const
{
    return depthWrite;
}

Material::BlendMode Material::getBlendMode() const
{
    return blendMode;
}

bool Material::getNormalMapping() const
{
    return normalMapping;
}

Texture* Material::getTexture() const
{
    return colorMap;
}

Texture* Material::getRefractionMap() const
{
    return refractionMap;
}

Texture* Material::getReflectionMap() const
{
    return reflectionMap;
}

Texture* Material::getNormalMap() const
{
    return normalMap;
}

RenderProgram* Material::getProgram() const
{
    return program;
}

glm::vec4 Material::getColor() const
{
    return color;
}

uint8_t Material::getShininess() const
{
    return shininess;
}

bool Material::getRefraction() const
{
    return refraction;
}

bool Material::getReflection() const
{
    return reflection;
}

float Material::getRefractCoef() const
{
    return refractCoef;
}

bool Material::getWireframe() const 
{
    return wireframe;
}

