#include "InputManager.h"

bool InputManager::isPressed(int key)
{
	return keybEvent[key];
}

glm::ivec2 InputManager::getMousePosition()
{ 
	return glm::ivec2(xpos, ypos); 
}

glm::ivec2 InputManager::getOldMousePosition()
{ 
	return glm::ivec2(oldxpos, oldypos); 
}