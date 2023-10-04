#define GLAD_ONLY_HEADERS
#include "common.h"
#include "GL1Render.h"
#include <iostream>

GL1Render::GL1Render(int width, int height) : Render(width, height)
{
    this->width = width;
    this->height = height;
    window = nullptr;
}

GL1Render::~GL1Render()
{
    if (window) glfwDestroyWindow(window);
}

int GL1Render::getHeigth() 
{
    return height;
}

void GL1Render::setHeigth(int heigth) 
{
    this->height = height;
}

void GL1Render::clearDisplay()
{
    // Limpiar buffer de color y profundidad
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GL1Render::swapBuffers()
{
    glfwSwapBuffers(window);
}

int GL1Render::getWidth() 
{
    return width;
}

void GL1Render::setWidth(int width) 
{
    this->width = width;
}

void GL1Render::init()
{
	// Inicializa GLFW
	if (!glfwInit())
    {
        std::cout << "ERROR GLFWINIT\n";
    }

    // Apple build
    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    #endif

    // Crear la ventana
    window = glfwCreateWindow(this->width, this->height, "OpenGL 1.0", nullptr, nullptr);

    if (!window)
    {
        std::cerr << "Error: no se pudo crear la ventana de GL1R." << std::endl;
        glfwTerminate();
    }
     
    glfwMakeContextCurrent(window); 
    glfwSwapInterval(0); // Disable vsync     
    gladLoadGL(glfwGetProcAddress);
    glfwWindowHint(GLFW_SAMPLES, 16); // antialiasing

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE); // antialiasing
}

void GL1Render::drawObject(Object* obj)
{
    obj->computeModelMatrix();
    for (const auto& mesh : obj->getMeshes())
    {
        // Color por mesh
        glm::vec4 color = mesh->getMaterial()->getColor();
        glColor3f(color.x, color.y, color.z);
        for (const auto& vert : *mesh->getVertices())
        {
            glm::vec4 position = obj->getModelMt() * vert.position;
            glVertex3f(position.x, position.y, position.z);
        }
    }
}

void GL1Render::drawObjects(const std::vector<Object*>& objs)
{
    glBegin(GL_TRIANGLES);
    for (const auto obj : objs) 
    {
        drawObject(obj);
    }
    glEnd();
}

bool GL1Render::isClosed()
{
    return bool(glfwWindowShouldClose(window));
}
