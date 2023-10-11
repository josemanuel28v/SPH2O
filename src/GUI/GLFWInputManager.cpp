#include "GUI/GLFWInputManager.h"

GLFWInputManager::GLFWInputManager()
{
    window = nullptr;
}

GLFWInputManager::~GLFWInputManager()
{
    if (window) glfwDestroyWindow(window);

    window = nullptr;
}

void GLFWInputManager::init()
{
    // Poner todas las teclas a false
    uint numKeys = sizeof(keybEvent) / sizeof(bool);
    for (uint i = 0; i < numKeys; i++)
    {
        keybEvent[i] = false;
    }

    glfwSetKeyCallback(window, keyManager);
    glfwSetCursorPosCallback(window, mouseManager);
    glfwSetMouseButtonCallback(window, mouseButtonManager);
    glfwSetFramebufferSizeCallback(window, resizeManager);
}

void GLFWInputManager::keyManager(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Todas las keys que recoge son el mayuscula
    switch (action)
    {
    case GLFW_PRESS:
        keybEvent[key] = true;
        break;

    case GLFW_RELEASE:
        keybEvent[key] = false;
        break;
    }
}

void GLFWInputManager::mouseManager(GLFWwindow* window, double xpos, double ypos) 
{
    oldxpos = GLFWInputManager::xpos;
    oldypos = GLFWInputManager::ypos;
    GLFWInputManager::xpos = (int)xpos;
    GLFWInputManager::ypos = (int)ypos;
}

void GLFWInputManager::mouseButtonManager(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        switch (action)
        {
            case GLFW_PRESS:
                rightButtonState = 1;
                break;

            case GLFW_RELEASE:
                rightButtonState = 0;
                break;
        } 
    }
}

void GLFWInputManager::disableCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void GLFWInputManager::enableCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void GLFWInputManager::resizeManager(GLFWwindow* window, int width, int height)
{
    GLFWInputManager::width = width;
    GLFWInputManager::height = height;
    resized = true;
}
