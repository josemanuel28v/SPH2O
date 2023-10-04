#pragma once
#include "RenderProgram.h"
#include "GLSLShader.h"
#include "Texture.h"

class Material
{
public:

	enum BlendMode
	{
		SOLID, ALPHA, ADD, MUL
	};

	virtual ~Material();

	virtual void loadPrograms(std::vector<std::string> fileNames) = 0;
	virtual void prepare() = 0;
	virtual void prepareInstanced() = 0;

	virtual void setTexturing(bool texturing);
	virtual void setLighting(bool lighting);
	virtual void setCulling(bool culling);
	virtual void setDepthWrite(bool depthWrite);
	virtual void setNormalMapping(bool normalMapping);
	virtual void setBlendMode(BlendMode blendMode);
	virtual void setTexture(Texture* colorMap);
	virtual void setRefractionMap(Texture* refractionMap);
	virtual void setReflectionMap(Texture* reflectionMap);
	virtual void setNormalMap(Texture* normalMap);
	virtual void setProgram(RenderProgram* program);
	virtual void setColor(glm::vec4 color);
	virtual void setShininess(uint8_t shininess);
	virtual void setRefraction(bool refraction);
	virtual void setReflection(bool reflection);
	virtual void setRefractCoef(float refractCoef);
	virtual void setWireframe(bool wireframe);

	virtual bool getTexturing() const;
	virtual bool getLighting() const;
	virtual bool getCulling() const;
	virtual bool getDepthWrite() const;
	virtual bool getNormalMapping() const;
	virtual BlendMode getBlendMode() const;
	virtual Texture* getTexture() const;
	virtual Texture* getRefractionMap() const;
	virtual Texture* getReflectionMap() const;
	virtual Texture* getNormalMap() const;
	virtual RenderProgram* getProgram() const;
	virtual glm::vec4 getColor() const;
	virtual uint8_t getShininess() const;
	virtual bool getRefraction() const;
	virtual bool getReflection() const;
	virtual float getRefractCoef() const;
	virtual bool getWireframe() const;

protected:

	RenderProgram* program = nullptr;
	Texture* colorMap = nullptr;
	Texture* normalMap = nullptr;
	Texture* refractionMap = nullptr;
	Texture* reflectionMap = nullptr;

	glm::vec4 color = glm::vec4(1.0f);
	uint8_t shininess = 50;
	float refractCoef = 0;

	bool wireframe = false;
	bool refraction = false;
	bool reflection = false;
	bool texturing = false;
	bool lighting = false;
	bool culling = false;
	bool depthWrite = false;
	bool normalMapping = false;
	BlendMode blendMode = BlendMode::SOLID;
};

