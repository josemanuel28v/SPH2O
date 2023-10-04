#pragma once

#include "../Common/numeric_types.h"
#include <vector>
#include <string>

// Structs para exportar e importar escenas en formato json
struct SceneInfo
{
    Real startTime;
    Real endTime;
    Real timeStep;
    Real fps;
    Real minTimeStep;
    Real maxTimeStep;
    Vector3r gravity;
    Real particleRadius;
    int simulationMethod;
    int boundaryMethod;
    Real stiffness;
    Real gamma;
    Real eta;
    Real etaV;
    Real cflFactor;
    uint minIterations;
    uint maxIterations;
    uint minIterationsV;
    uint maxIterationsV;
};

struct BlockInfo
{
    Vector3r min; 
    Vector3r max; 
};

struct EmitterInfo
{
    uint type; 
    uint numParticles; 
    Vector3r r;
    Real v; 
    Quat4r rot; 
    Real startTime; 
    Real width; 
    Real height;
    Real spacing;
};

struct Fluid // Representa un fluid object en la misma fase
{
    std::vector<BlockInfo> fluidBlocks;
    std::vector<EmitterInfo> emitters;
    //std::vector<> geometries;

    Real viscosity;
    Real boundaryViscosity;
    Real surfaceTension;
    Real adhesion;
    Real density0;
};

struct FluidInfo
{
    std::vector<Fluid> fluids;

    int viscosityMethod;
    int surfaceTensionMethod;
    int adhesionMethod;
};

struct Sphere
{
    Vector3r pos;
    Real radius;
};

struct Geometry
{
    std::string path;
    Real spacing;
};

struct Boundary // Representa un boundary object
{
    std::vector<std::pair<BlockInfo, bool>> box;
    std::vector<std::pair<Sphere, bool>> sphere;
    std::vector<Geometry> geometry;

    Real normalFct;
    Real tangFct;
};

struct BoundaryInfo
{
    std::vector<Boundary> boundaries;
};

struct SimulationInfo
{
    SceneInfo sceneData;
    FluidInfo fluidData;
    BoundaryInfo boundaryData;
};