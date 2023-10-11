#pragma once
#include "Texture.h"
#include "pugixml.hpp"

struct color32_t {
	unsigned char r, g, b, a;
};

class GLTexture: public Texture
{
public:
	GLTexture();
	GLTexture(std::string fileName);
	GLTexture(std::string left, std::string right, std::string front, std::string back, std::string top, std::string bottom);
	virtual void bind(uint) override;
	virtual void load(std::string fileName) override;
	virtual void load(std::string left, std::string right, std::string front, std::string back, std::string top, std::string bottom) override;
	virtual GLuint getID() const override;

};

