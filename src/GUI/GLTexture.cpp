#include "GLTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLTexture::GLTexture() : Texture()
{}

GLTexture::GLTexture(std::string fileName) : GLTexture()
{
	load(fileName);
}

GLTexture::GLTexture(std::string left, std::string right, std::string front, std::string back, std::string top, std::string bottom) : GLTexture()
{
	load(left, right, front, back, top, bottom);
}

GLuint GLTexture::getID() const
{
    return this->id;
}

void GLTexture::bind(uint textUnit)
{
	switch (this->type)
	{
	case COLOR2D:
		glActiveTexture(GL_TEXTURE0 + textUnit);
		glBindTexture(GL_TEXTURE_2D, id);
		break;

	case COLOR3D:
		glActiveTexture(GL_TEXTURE0 + textUnit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		break;
	}
}

void GLTexture::load(std::string fileName) 
{
	int comp = 0;
	color32_t* pixels = (color32_t*)stbi_load(fileName.c_str(), &size.x, &size.y, &comp, 4);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	stbi_image_free(pixels);

	this->type = COLOR2D;
}

void GLTexture::load(std::string left, std::string right, std::string front, std::string back, std::string top, std::string bottom)
{
	// Se asume que todas las im�genes del cubemap tienen las mismas dimensiones

	int comp = 0;

	color32_t* leftPixels = (color32_t*)stbi_load(left.c_str(), &size.x, &size.y, &comp, 4);
	color32_t* rightPixels = (color32_t*)stbi_load(right.c_str(), &size.x, &size.y, &comp, 4);
	color32_t* frontPixels = (color32_t*)stbi_load(front.c_str(), &size.x, &size.y, &comp, 4);
	color32_t* backPixels = (color32_t*)stbi_load(back.c_str(), &size.x, &size.y, &comp, 4);
	color32_t* topPixels = (color32_t*)stbi_load(top.c_str(), &size.x, &size.y, &comp, 4);
	color32_t* bottomPixels = (color32_t*)stbi_load(bottom.c_str(), &size.x, &size.y, &comp, 4);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, leftPixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rightPixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bottomPixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, topPixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, backPixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, frontPixels);

	stbi_image_free(leftPixels);
	stbi_image_free(rightPixels);
	stbi_image_free(bottomPixels);
	stbi_image_free(topPixels);
	stbi_image_free(backPixels);
	stbi_image_free(frontPixels);

	this->type = COLOR3D;
}
