#include "GUI/common.h"
#include "GUI/System.h"
#include "GUI/CustomParticle.h"
#include "GUI/CameraKeyboard.h"
#include "GUI/Light.h"
#include "GUI/FactoryEngine.h"
#include "GUI/Cube.h"
#include "GUI/Sphere3D.h"
#include "GUI/ObjModel.h"
#include "SPH2O/SPHSimulation.h"
#include "SPH2O/InputParser.h"
#include "SPH2O/Utilities.h"

void setupGraphicSystem(SPHSimulation*);

int main(int argc, char *argv[])
{
	/////////////////////////////////////////////////////////
	// -c Especificar la ruta al fichero de configuracion
	// -o Activar la salida de datos de la simulacion
	// -g Activar la visualizacion grafica de la simulacion
	// -f Especificar el frame del ultimo fichero de salida
	/////////////////////////////////////////////////////////

	InputParser(argc, argv);
	SPHSimulation* sim = SPHSimulation::getCurrent();

	// Almacenar datos de simulación
	if (InputParser::cmdOptionExists("-o"))
	{
		std::string activeOutput = InputParser::getCmdOption("-o");

		if (activeOutput == "true")
			sim->activateSave(true);
		else if (activeOutput == "false")
			sim->activateSave(false);
	}
	else
		sim->activateSave(false);

	// Importar escena
	if (InputParser::cmdOptionExists("-c"))
	{
		if (!sim->importScene(InputParser::getCmdOption("-c"))) 
		{
			ERROR("Cannot import scene!");
			return 0;
		}
	}
	else
	{
		if (!sim->importScene("../../data/example_scene/example_scene.json")) 
		{
			ERROR("Cannot import example scene!");
			return 0;
		}
	}

	// Comprobar si se proporciona un frame con el estado del fluido
	if (InputParser::cmdOptionExists("-f"))
	{
		std::string frame = InputParser::getCmdOption("-f");

		if (Utilities::isNumber(frame))
		{
			if (!sim->importFluidState(std::stoi(frame)))
			{
				ERROR("Cannot import fluid state ", frame);
				return 0;
			}
		}
	}

	// Activar o no el modo gráfico (se activa por defecto)
	if (InputParser::cmdOptionExists("-g"))
	{
		if (InputParser::getCmdOption("-g") == "false")
		{
			sim->init();
			sim->run();
		}
		else if (InputParser::getCmdOption("-g") == "true")
		{
			setupGraphicSystem(sim);
		}
	}
	else
		setupGraphicSystem(sim);

	return 0;
}

void setupGraphicSystem(SPHSimulation* sim)
{
	// Select backends
	FactoryEngine::SetSelectedGraphicsBackend(FactoryEngine::GL4);
	FactoryEngine::SetSelectedInputBackend(FactoryEngine::GLFW);

	// Init system 
	System::initSystem();

	// Camera setup
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 position(-0.031391, 1.417409, 2.446536); 
	glm::vec3 lookAt(-0.038991, 0.884833, 0.392759);
	Camera* camera = new CameraKeyboard(Camera::PERSPECTIVE, position, up, lookAt);

	glm::vec3 lightPos(0.104284, 5.777095, 3.671279);
	glm::vec3 lightColor(1.0f);
	float intensity = 2.0f;
	float linearAtt = 0.01f;
	Light* light = new Light(lightPos, lightColor, Light::POINT, intensity, linearAtt);

	// SPHSimulation  
	GraphicSimulationSystem* simSys = new GraphicSimulationSystem(sim);
	simSys->setRotation(Vector4r(-M_PI_2, 0.0, 0.0, 0.0));
	simSys->addPrototype(new CustomParticle(sim->getParticleRadius()));
	simSys->addPrototype(new CustomParticle(sim->getParticleRadius()));
	simSys->addPrototype(new CustomParticle(sim->getParticleRadius()));

	// System
	System::setSimulationSystem(simSys);
	System::setCamera(camera);
	System::addLight(light);
	System::setAmbient(glm::vec3(0.2f));
	System::mainLoop();
	System::releaseMemory();
}

