#include "GUI/FactoryEngine.h" 

int FactoryEngine::selectedGraphicsBackend = 0;
int FactoryEngine::selectedInputBackend = 0;

void FactoryEngine::SetSelectedGraphicsBackend(int backend) 
{
    selectedGraphicsBackend = backend;
}

void FactoryEngine::SetSelectedInputBackend(int backend) 
{
    selectedInputBackend = backend;
}

int FactoryEngine::GetSelectedGraphicsBackend() 
{
    return selectedGraphicsBackend;
}

int FactoryEngine::GetSelectedInputBackend()
{
    return selectedInputBackend;
}

Render* FactoryEngine::getNewRender() 
{
    switch (selectedGraphicsBackend)
    {
    case GraphicsBackend::GL1:
        return new GL1Render(640, 480);
    case GraphicsBackend::GL4:
        return new GL4Render(640, 480);
    default:
        return new GL1Render(640, 480);
    }
}

InputManager* FactoryEngine::getNewInputManager() {

    switch (selectedGraphicsBackend)
    {
    case GraphicsBackend::GL1:
        return new GLFWInputManager();

    default:
        return new GLFWInputManager();
    }
}

Material* FactoryEngine::getNewMaterial()
{
    switch (selectedGraphicsBackend)
    {
    case GraphicsBackend::GL4: 
        return new GLSLMaterial();
    default:
        return nullptr;  
    } 
}

Texture* FactoryEngine::getNewTexture()
{
    switch (selectedGraphicsBackend)
    {
    case GraphicsBackend::GL1:
        return new GLTexture();

    case GraphicsBackend::GL4:
        return new GLTexture();

    default:
        return new GLTexture();
    }
}

