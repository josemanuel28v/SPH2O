#pragma once
#define GLAD_ONLY_HEADERS
#include "InputManager.h"
#include "common.h"

class GLFWInputManager: public InputManager
{
	GLFWwindow* window = nullptr;
public:
	GLFWInputManager();
	virtual ~GLFWInputManager();
	virtual void init() override;

	static void keyManager(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseManager(GLFWwindow* window, double xpos, double ypos);
	static void mouseButtonManager(GLFWwindow* window, int button, int action, int mods);
	static void resizeManager(GLFWwindow* window, int width, int height);

	// Para poder inicializar las funciones de callback pasandole la ventana
	void setWindow(GLFWwindow* window) { this->window = window; }
	void setWindowTitle(const std::string& title) override { glfwSetWindowTitle(window, title.c_str()); }
	
	void disableCursor() override;
	void enableCursor() override;

};

