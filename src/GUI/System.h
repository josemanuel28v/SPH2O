#pragma once

#include "Camera.h"
#include "Render.h"
#include "InputManager.h"
#include "Object.h"
#include "Light.h"
#include "GraphicSimulationSystem.h"

class TimeManager 
{
	const uint NUM_SAMPLES = 100;
	double newTime = 0;
	double deltaTime = 0;
	double meanDeltaTime = 0;
	double lastTime = 0;
	std::vector<double> samples;
	uint index = 0;

public:

	TimeManager()
	{
		samples.resize(NUM_SAMPLES);
	}

	void update() 
	{
		newTime = glfwGetTime();
		deltaTime = newTime - lastTime;
		lastTime = newTime;

		if (index < NUM_SAMPLES)
		{
			samples[index] = deltaTime;
			meanDeltaTime = deltaTime;
			index++;
		}
		else
		{
			meanDeltaTime = 0.0;
			for (uint i = 1; i < NUM_SAMPLES; ++i)
			{
				samples[i - 1] = samples[i]; 
				meanDeltaTime += samples[i];
			}
			samples[NUM_SAMPLES - 1] = deltaTime;
			meanDeltaTime += deltaTime;
			meanDeltaTime /= NUM_SAMPLES;
		}
	}

	double getDeltaTime()
	{
		return deltaTime;
	}

	double getMeanDeltaTime()
	{
		return meanDeltaTime;
	}

	double getTime()
	{
		return newTime;
	}
};

class System
{
public:

	static void initSystem();
	static void mainLoop();
	static void releaseMemory();

	static void addLight(Light* light);
	static void addObject(Object* obj);

	static void removeLight(int lightIdx);
	static void removeObject(int objectIdx);

	static void setCamera(Camera* camera) { System::camera = camera; }
	static void setModelMatrix(glm::mat4 modelMatrix) { System::modelMatrix = modelMatrix; }
	static void setAmbient(const glm::vec3& ambient) { System::ambient = ambient; }
	static void setSimulationSystem(GraphicSimulationSystem* simSystem) { System::simSystem = simSystem; }

	static Camera* getCamera() { return System::camera; }
	static glm::mat4 getModelMatrix() { return System::modelMatrix; }
	static const glm::vec3 getAmbient() { return System::ambient; }
	static GraphicSimulationSystem* getSimulationSystem() { return simSystem; }
	static InputManager* getInputManager() { return inputManager; }
	static std::vector<Light*>& getLights() { return lights; }

protected:

	static void events();
	static void coordinateSystem();

	inline static Camera* camera = nullptr;
	inline static Render* render = nullptr;
	inline static InputManager* inputManager = nullptr;
	inline static std::vector<Object*> objects;
	inline static std::vector<Light*> lights;
	inline static GraphicSimulationSystem* simSystem = nullptr;
	inline static bool exit, pause = true;
	inline static glm::mat4 modelMatrix;
	inline static glm::vec3 ambient;
	
};

