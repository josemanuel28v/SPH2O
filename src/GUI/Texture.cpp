#include "GUI/Texture.h"

glm::ivec2 Texture::getSize() const 
{
    return size;
}

void Texture::setSize(glm::ivec2 size)
{
    this->size = size;
}

