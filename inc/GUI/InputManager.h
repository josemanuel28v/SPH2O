#pragma once

#include "Common/numeric_types.h"

class InputManager
{
public:

	virtual void init() = 0;
	virtual bool isPressed(int key);
	virtual glm::ivec2 getMousePosition();
	virtual glm::ivec2 getOldMousePosition();
	virtual int getRightButtonState() { return rightButtonState; }

	virtual void disableCursor() = 0;
	virtual void enableCursor() = 0;

	virtual glm::ivec2 getWindowSize() { return glm::ivec2(width, height); }
	virtual bool isWindowResized() { return resized; }
	virtual void setWindowResized() { resized = false; }
	virtual void setWindowTitle(const std::string& title) = 0;

protected:
	inline static bool keybEvent[512];
	inline static int xpos, ypos, oldxpos, oldypos = 0;
	inline static int rightButtonState = -1;
	inline static int width, height;
	inline static bool resized = false;
};

