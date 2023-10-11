#pragma once
#include "vertex_t.h"
#include "Material.h"

class GLSLMaterial: public Material
{
public:

	GLSLMaterial();
	virtual void loadPrograms(std::vector<std::string> fileNames) override;
	virtual void prepare() override;
	virtual void prepareInstanced() override;
};

