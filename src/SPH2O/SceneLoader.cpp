#include "SceneLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "../Common/numeric_types.h"
#include "Logger.h"
#include "SPHSimulation.h"

bool SceneLoader::readConfiguration(SimulationInfo& simData, const std::string& path)
{
    json config;
    std::ifstream file(path);

    if (file.is_open())
    {
        file >> config;

        INFO("Reading general configuration...");
        readSceneInfo(simData, config);

        INFO("Reading fluid data...");
        readFluidInfo(simData, config);        

        INFO("Reading boundary data...");
        readBoundaryInfo(simData, config);

        INFO("Read!");

        return true;
    }
    else
    {
        ERROR("Cannot read ", path);
        return false;
    }
}

void SceneLoader::readSceneInfo(SimulationInfo & simData, const json& config)
{
    SceneInfo& sceneData = simData.sceneData;

    sceneData.startTime = config["Configuration"]["startTime"];
    sceneData.endTime = config["Configuration"]["endTime"];
    sceneData.timeStep = config["Configuration"]["timeStep"];
    sceneData.fps = config["Configuration"]["fps"];
    sceneData.minTimeStep = config["Configuration"]["minTimeStep"];
    sceneData.maxTimeStep = config["Configuration"]["maxTimeStep"];
    sceneData.cflFactor = config["Configuration"]["cflFactor"];
    sceneData.particleRadius = config["Configuration"]["particleRadius"];
    sceneData.boundaryMethod = config["Configuration"]["boundaryMethod"];
    sceneData.simulationMethod = config["Configuration"]["simulationMethod"];
    sceneData.gravity = Vector3r(config["Configuration"]["gravity"][0],
                                    config["Configuration"]["gravity"][1],
                                    config["Configuration"]["gravity"][2]);

    if (sceneData.simulationMethod == SPHSimulation::WCSPH_METHOD)
    {
        sceneData.stiffness = config["Configuration"]["stiffness"];
        sceneData.gamma = config["Configuration"]["gamma"];
    }
    else if (sceneData.simulationMethod == SPHSimulation::PCISPH_METHOD)
    {
        sceneData.eta = config["Configuration"]["eta"];
        sceneData.minIterations = config["Configuration"]["minIterations"];
        sceneData.maxIterations = config["Configuration"]["maxIterations"];
    }
    else if (sceneData.simulationMethod == SPHSimulation::DFSPH_METHOD)
    {
        sceneData.eta = config["Configuration"]["eta"];
        sceneData.etaV = config["Configuration"]["etaV"];
        sceneData.minIterations = config["Configuration"]["minIterations"];
        sceneData.maxIterations = config["Configuration"]["maxIterations"];
        sceneData.minIterationsV = config["Configuration"]["minIterationsV"];
        sceneData.maxIterationsV = config["Configuration"]["maxIterationsV"];
    }
}

void SceneLoader::readFluidInfo(SimulationInfo & simData, const json& config)
{
    FluidInfo& fluidData = simData.fluidData;

    uint numFluidModels = static_cast<uint>(config["Fluid"]["FluidModel"].size());
    fluidData.fluids.resize(numFluidModels);
    fluidData.viscosityMethod = config["Fluid"]["viscosityMethod"];
    fluidData.surfaceTensionMethod = config["Fluid"]["surfaceTensionMethod"];
    fluidData.adhesionMethod = config["Fluid"]["adhesionMethod"];

    for (uint i = 0; i < numFluidModels; ++i)
    {
        fluidData.fluids[i].density0 = config["Fluid"]["FluidModel"][i]["density0"];

        if (fluidData.viscosityMethod > -1)
        {
            fluidData.fluids[i].viscosity = config["Fluid"]["FluidModel"][i]["viscosity"];
            fluidData.fluids[i].boundaryViscosity = config["Fluid"]["FluidModel"][i]["boundaryViscosity"];
        }

        if (fluidData.surfaceTensionMethod > -1)
            fluidData.fluids[i].surfaceTension = config["Fluid"]["FluidModel"][i]["surfaceTension"];
        
        if (fluidData.adhesionMethod > -1)
            fluidData.fluids[i].adhesion = config["Fluid"]["FluidModel"][i]["adhesion"];

        uint numFluidBlocks = static_cast<uint>(config["Fluid"]["FluidModel"][i]["fluidBlocks"].size());
        fluidData.fluids[i].fluidBlocks.resize(numFluidBlocks);
        
        if (config["Fluid"]["FluidModel"][i].find("emitters") != config["Fluid"]["FluidModel"][i].end())
        {
            uint numEmitters = static_cast<uint>(config["Fluid"]["FluidModel"][i]["emitters"].size());
            fluidData.fluids[i].emitters.resize(numEmitters);
        }
        
        // geometrias

        for (uint j = 0; j < numFluidBlocks; ++j)
        {
            fluidData.fluids[i].fluidBlocks[j].min = Vector3r(config["Fluid"]["FluidModel"][i]["fluidBlocks"][j]["min"][0],
                                                                config["Fluid"]["FluidModel"][i]["fluidBlocks"][j]["min"][1],
                                                                config["Fluid"]["FluidModel"][i]["fluidBlocks"][j]["min"][2]);

            fluidData.fluids[i].fluidBlocks[j].max = Vector3r(config["Fluid"]["FluidModel"][i]["fluidBlocks"][j]["max"][0],
                                                                config["Fluid"]["FluidModel"][i]["fluidBlocks"][j]["max"][1],
                                                                config["Fluid"]["FluidModel"][i]["fluidBlocks"][j]["max"][2]);
        }

        for (uint j = 0; j < fluidData.fluids[i].emitters.size(); ++j)
        {
            EmitterInfo& emitter = fluidData.fluids[i].emitters[j];

            const auto& position = config["Fluid"]["FluidModel"][i]["emitters"][j]["position"];
            const auto& rotation = config["Fluid"]["FluidModel"][i]["emitters"][j]["rotation"];

            emitter.r = Vector3r(position[0], position[1], position[2]);
            emitter.rot = Quat4r(rotation[0], rotation[1], rotation[2], rotation[3]);
            emitter.v = config["Fluid"]["FluidModel"][i]["emitters"][j]["velocity"];
            emitter.numParticles = config["Fluid"]["FluidModel"][i]["emitters"][j]["numParticles"];
            emitter.startTime = config["Fluid"]["FluidModel"][i]["emitters"][j]["startTime"];
            emitter.type = config["Fluid"]["FluidModel"][i]["emitters"][j]["type"];
            emitter.width = config["Fluid"]["FluidModel"][i]["emitters"][j]["width"];
            emitter.height = config["Fluid"]["FluidModel"][i]["emitters"][j]["height"];
            emitter.spacing = config["Fluid"]["FluidModel"][i]["emitters"][j]["spacing"];
        }
    }
}

void SceneLoader::readBoundaryInfo(SimulationInfo & simData, const json& config)
{
    if (config.find("Boundary") == config.end()) return;

    BoundaryInfo& boundaryData = simData.boundaryData;
    SceneInfo& sceneData = simData.sceneData;    

    uint numBoundaries = static_cast<uint>(config["Boundary"].size());
    boundaryData.boundaries.resize(numBoundaries);

    for (uint i = 0; i < numBoundaries; ++i)
    {
        uint numBox = static_cast<uint>(config["Boundary"][i]["box"].size());
        uint numSphere = static_cast<uint>(config["Boundary"][i]["sphere"].size());
        uint numGeometry = static_cast<uint>(config["Boundary"][i]["geometry"].size());

        boundaryData.boundaries[i].box.resize(numBox);
        boundaryData.boundaries[i].sphere.resize(numSphere);
        boundaryData.boundaries[i].geometry.resize(numGeometry);

        if (sceneData.boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD) 
        {
            boundaryData.boundaries[i].normalFct = config["Boundary"][i]["normalFct"];
            boundaryData.boundaries[i].tangFct = config["Boundary"][i]["tangFct"];
        }

        // Cubes
        for (uint j = 0; j < numBox; ++j)
        {
            if (sceneData.boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD) 
                boundaryData.boundaries[i].box[j].second = config["Boundary"][i]["box"][j]["inverted"];

            boundaryData.boundaries[i].box[j].first.min = Vector3r(config["Boundary"][i]["box"][j]["min"][0],
                                                                config["Boundary"][i]["box"][j]["min"][1],
                                                                config["Boundary"][i]["box"][j]["min"][2]);

            boundaryData.boundaries[i].box[j].first.max = Vector3r(config["Boundary"][i]["box"][j]["max"][0],
                                                                config["Boundary"][i]["box"][j]["max"][1],
                                                                config["Boundary"][i]["box"][j]["max"][2]);
        }

        // Spheres
        for (uint j = 0; j < numSphere; ++j)
        {
            if (sceneData.boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD) 
                boundaryData.boundaries[i].sphere[j].second = config["Boundary"][i]["sphere"][j]["inverted"];

            boundaryData.boundaries[i].sphere[j].first.pos = Vector3r(config["Boundary"][i]["sphere"][j]["pos"][0],
                                                                config["Boundary"][i]["sphere"][j]["pos"][1],
                                                                config["Boundary"][i]["sphere"][j]["pos"][2]);

            boundaryData.boundaries[i].sphere[j].first.radius = config["Boundary"][i]["sphere"][j]["radius"];                                           
        }

        // Geometries
        for (uint j = 0; j < numGeometry; ++j)
        {
            boundaryData.boundaries[i].geometry[j].path = config["Boundary"][i]["geometry"][j]["path"];
            boundaryData.boundaries[i].geometry[j].spacing = config["Boundary"][i]["geometry"][j]["spacing"];
        }
    }
}

void SceneLoader::writeFluid()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    std::string filename = sim->getName();
    uint frame = sim->getFrame();

    std::string jsonext = ".json";
    std::string num = std::to_string(frame);
    
    uint iters = MAX_DIGITS - static_cast<uint>(num.length());
    for (uint i = 0; i < iters; ++i)
        num = "0" + num;

    filename.insert(filename.length() - jsonext.length(), "_" + num);

    json state;
    std::ofstream file(filename);

    if (file.is_open())
    {
        uint numFluids = sim->numberFluidModels();
        uint totalParticles = 0;
        uint totalActiveParticles = 0;
        for (uint fObjId = 0; fObjId < numFluids; ++fObjId)
        {
            totalParticles += sim->getFluidModel(fObjId)->getSize();
            totalActiveParticles += sim->getFluidModel(fObjId)->getNumActiveParticles();
        }

        state["numParticles"] = totalParticles;
        state["numActiveParticles"] = totalActiveParticles;
        state["time"] = sim->getTime();
        state["timeStep"] = sim->getTimeStep();

        uint pid = 0;
        for (uint fObjId = 0; fObjId < numFluids; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            uint numParticles = fObj->getSize();

            for (uint i = 0; i < numParticles; ++i)
            {
                const Vector3r& pos = fObj->getPosition(i);
                state["positions"][pid][0] = pos.x;
                state["positions"][pid][1] = pos.y;
                state["positions"][pid][2] = pos.z;

                const Vector3r& vel = fObj->getVelocity(i);
                state["velocities"][pid][0] = vel.x;
                state["velocities"][pid][1] = vel.y;
                state["velocities"][pid][2] = vel.z;

                ++pid;
            }
        }

        file << state;

        file.close();
    }
    else
    {
        ERROR("Cannot write fluid state");
    }
}
