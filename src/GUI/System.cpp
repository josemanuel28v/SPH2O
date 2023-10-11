#include "GUI/System.h"
#include "GUI/GL1Render.h"
#include "GUI/GL4Render.h"
#include "GUI/GLFWInputManager.h"
#include "GUI/FactoryEngine.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

void System::initSystem()
{
	render = FactoryEngine::getNewRender();
	inputManager = FactoryEngine::getNewInputManager();

	render->init();

	// Setear la ventana de GL1Render o GL4Render a GLFWInputManager
	if (FactoryEngine::GetSelectedGraphicsBackend() == FactoryEngine::GL1 && FactoryEngine::GetSelectedInputBackend() == FactoryEngine::GLFW)
	{
		GL1Render* gl1Render = dynamic_cast<GL1Render*>(render);
		GLFWInputManager* glfwInput = dynamic_cast<GLFWInputManager*>(inputManager);

		glfwInput->setWindow(gl1Render->getWindow());
	}
	else if (FactoryEngine::GetSelectedGraphicsBackend() == FactoryEngine::GL4 && FactoryEngine::GetSelectedInputBackend() == FactoryEngine::GLFW)
	{
		GL4Render* gl4Render = dynamic_cast<GL4Render*>(render);
		GLFWInputManager* glfwInput = dynamic_cast<GLFWInputManager*>(inputManager);

		glfwInput->setWindow(gl4Render->getWindow());
	}

	inputManager->init();
}

void System::addObject(Object* obj)
{
	objects.push_back(obj);
}

void System::removeObject(int objectIdx)
{
	delete objects[objectIdx];
	objects.erase(objects.begin() + objectIdx);
}

// TODO: mover al render
void captureFramebufferToPNG(const char* filename, int width, int height) 
{
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filename, width, height, nrChannels, buffer.data(), stride);
}

void System::mainLoop()
{
	TimeManager t;

	for (auto obj : objects)
	{
		render->setupObject(obj);
	}

	if (simSystem)
	{
		simSystem->init();
		for (uint i = 0; i < simSystem->getSize(); ++i)
		{
			SimulationObject* simObj = simSystem->getSimObject(i);
			render->setupObject(simSystem->getPrototype(i), static_cast<uint>(simObj->getSize()));
		}
		simSystem->deferredInit();
	}

	int frame = 1;
	while (!exit) 
	{
		inputManager->setWindowTitle("FPS: " + std::to_string((round(1.0f / t.getMeanDeltaTime() * 1000.0f) / 1000.0f)));

		// Limpiar buffers
		render->clearDisplay();

		// Actualizar time manager
		t.update();

		// Actualizar camara
		camera->step(t.getDeltaTime());

		// Actualzar objetos
		for (auto object : objects) 
		{
			object->step(t.getDeltaTime());
		}

		// Actualizar luces
		for (auto light : lights)
		{
			light->step(t.getDeltaTime());
		}

		// Dibujar objetos
		render->drawObjects(objects);

		// Dibujar simulation system
		bool save = false;
		if (simSystem)
		{
			if (!pause) 
			{
				save = simSystem->step();
			}
			simSystem->updateScalarField();
			simSystem->computeModelMatrix();
			setModelMatrix(simSystem->getModelMt());
			for (uint i = 0; i < simSystem->getSize(); ++i)
			{
				SimulationObject* simObj = simSystem->getSimObject(i);
				render->drawObject(simSystem->getPrototype(i), static_cast<uint>(simObj->getActiveCount()), simObj->getPositions(), simSystem->getScalarField(i));
			}

			// Debug draw light
			std::vector<Real> sf = {0.0};
			std::vector<glm::vec4> r = {lights[0]->getPosition()};
			lights[0]->computeModelMatrix();
			setModelMatrix(lights[0]->getModelMt());
			render->drawObject(simSystem->getPrototype(0), 1, r.data(), sf.data());
		}

		render->swapBuffers();

		// if (save)
		// {
		// 	std::string filename = std::to_string(frame) + ".png";
		// 	captureFramebufferToPNG(filename.c_str(), 1920, 1080);
		// 	frame++;
		// }

		// Gestionar eventos
		glfwPollEvents();
		events();
	}	 
}

void System::addLight(Light* light)
{
	lights.push_back(light);
}

void System::removeLight(int lightIdx)
{
	delete lights[lightIdx];
	lights.erase(lights.begin() + lightIdx);
}

void System::releaseMemory()
{
	if (camera) delete camera;
	if (render) delete render;
	if (inputManager) delete inputManager;

	// Liberar memoria del contenido del vector objects
	for (Object* obj : objects)
	{
		delete obj;
	}

	// Liberar memoria del contenido del vector lights
	for (Light* light : lights)
	{
		delete light;
	}

	// Vaciar vectores
	objects.clear();
	lights.clear();

	camera = nullptr;
	render = nullptr;
	inputManager = nullptr;
}

void System::events()
{
    static bool pressed = false;
    static bool boundaryPressed = false;

    if (inputManager->isPressed('E') || render->isClosed())
    {
        exit = true;
    }

    if (inputManager->isPressed('P') && !pressed)
    {
        pause = !pause;
        pressed = true;
    }
    else if (!inputManager->isPressed('P'))
    {
        pressed = false;
    }

    // if (inputManager->isPressed('B') && !boundaryPressed)
    // {
    //     drawBoundary = !drawBoundary;
    //     boundaryPressed = true;
    // }
    // else if (!inputManager->isPressed('B'))
    // {
    //     boundaryPressed = false;
    // }
    // if (inputManager->isPressed('R'))
    // {
    //     if (simSystem) simSystem->reset();
    // }

    if (inputManager->isWindowResized())
    {
        glm::ivec2 size = inputManager->getWindowSize();
        glViewport(0, 0, size.x, size.y);
        inputManager->setWindowResized();
        camera->setAspect((float)size.x / size.y);
        camera->computeProjectionMatrix();
    }	

    if (inputManager->isPressed('I'))
    {
		Light* light = lights[0];
        auto position = light->getPosition();
        position.y += 0.001f;
        light->setPosition(position);
    }

    if (inputManager->isPressed('K'))
    {
		Light* light = lights[0];
        auto position = light->getPosition();
        position.y -= 0.001f;
        light->setPosition(position);
    }

    if (inputManager->isPressed('L'))
    {
		Light* light = lights[0];
        auto position = light->getPosition();
        position.x += 0.001f;
        light->setPosition(position);
    }

    if (inputManager->isPressed('J'))
    {
		Light* light = lights[0];
        auto position = light->getPosition();
        position.x -= 0.001f;
        light->setPosition(position);
    }

    if (inputManager->isPressed('U'))
    {
		Light* light = lights[0];
        auto position = light->getPosition();
        position.z += 0.001f;
        light->setPosition(position);
    }

    if (inputManager->isPressed('O'))
    {
		Light* light = lights[0];
        auto position = light->getPosition();
        position.z -= 0.001f;
        light->setPosition(position);
    }
}

void System::coordinateSystem() 
{
	// glm::vec3 a(0,0,0);
	// glm::vec3 b(2,0,0);
	// glm::vec3 c(0,2,0);
	// glm::vec3 d(0,0,2);

	// float diffcolor [4] = {1,0,0,1};
	// float speccolor [4] = {1,1,1,1};
	// glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, diffcolor);
	// glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, diffcolor);
	// glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, speccolor);
	// glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 100.0);
	// glLineWidth (2);

	// glBegin (GL_LINES);
	// 	glVertex3fv (&a[0]);
	// 	glVertex3fv (&b[0]);
	// glEnd ();

	// float diffcolor2[4] = { 0, 1, 0, 1 };
	// glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, diffcolor2);
	// glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, diffcolor2);

	// glBegin (GL_LINES);
	// 	glVertex3fv (&a[0]);
	// 	glVertex3fv (&c[0]);
	// glEnd ();

	// float diffcolor3[4] = { 0, 0, 1, 1 };
	// glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, diffcolor3);
	// glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, diffcolor3);

	// glBegin (GL_LINES);
	// 	glVertex3fv (&a[0]);
	// 	glVertex3fv (&d[0]);
	// glEnd ();
	// glLineWidth (1);
}
