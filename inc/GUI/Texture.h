#pragma once
#define GLAD_ONLY_HEADERS
#include "common.h"
#include <string>
#include "Common/numeric_types.h"

class Texture
{
public:

	enum TextureType
	{
		COLOR2D = 0, COLOR3D = 1
	};

	Texture() { textUnit = textUnitCounter++; }
	virtual void bind(uint) = 0;
	virtual void load(std::string fileName) = 0;
	virtual void load(std::string left, std::string right, std::string front, std::string back, std::string top, std::string bottom) = 0;
	
	virtual GLuint getID() const = 0;
	glm::ivec2 getSize() const;
	uint getTextureUnit() const { return textUnit; }
	TextureType getType() const { return type; }

	void setSize(glm::ivec2 size);
	void setType(TextureType type) { this->type = type; }

protected:
	TextureType type;
	GLuint id;
	glm::ivec2 size;
	unsigned textUnit;
	inline static unsigned textUnitCounter = 0;
};

