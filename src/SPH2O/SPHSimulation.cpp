#include "SPHSimulation.h" 
#include "WCSPHSolver.h"
#include "PCISPHSolver.h"
#include "DFSPHSolver.h"
#include "Poly6.h"
#include "Spiky.h"
#include "ViscoK.h"
#include "Cohesion.h"
#include "Adhesion.h"
#include "CubicSpline.h"
#include "Logger.h"
#include "SceneLoader.h"
#include "AkinciBoundaryObject.h"
#include "PCISPHBoundaryObject.h"
#include "CubeBoundaryObject.h"
#include <fstream>

SPHSimulation* SPHSimulation::current = nullptr;

const int SPHSimulation::WCSPH_METHOD = 0;
const int SPHSimulation::PCISPH_METHOD = 1;
const int SPHSimulation::DFSPH_METHOD = 2;

const int SPHSimulation::CUBE_BOUNDARY_METHOD = 0;
const int SPHSimulation::PCISPH_BOUNDARY_METHOD = 1;
const int SPHSimulation::AKINCI_BOUNDARY_METHOD = 2;

const int SPHSimulation::STANDARD_VISCOSITY_METHOD = 0;
const int SPHSimulation::ARTIFICIAL_VISCOSITY_METHOD = 1;
const int SPHSimulation::XSPH_VISCOSITY_METHOD = 2;
const int SPHSimulation::AKINCI_SURFACE_TENSION_METHOD = 3;
const int SPHSimulation::AKINCI_ADHESION_METHOD = 4;

SPHSimulation::SPHSimulation()
{
    // Inicializar  parametros por defecto
    activeSave = false;
    name = "";

    current_method = -1;
    current_bh_method = CUBE_BOUNDARY_METHOD;
    current_visco_method = -1;

    current_kernel_method = -1;
    current_gradient_method = -1;
    current_laplacian_method = -1;

    solver = nullptr;
    grid = nullptr;

    grid = new HashTable();
}

SPHSimulation::~SPHSimulation()
{
    // Eliminar punteros
    delete solver;
    delete grid;
    delete current;
}

SPHSimulation* SPHSimulation::getCurrent()
{
    if (current == nullptr)
        current = new SPHSimulation();

    return current;
}

void SPHSimulation::setCurrent(SPHSimulation* sim)
{
    current = sim;
}

bool SPHSimulation::hasCurrent()
{
    return current != nullptr;
}

void SPHSimulation::reset()
{
    if (name != "")
    {
        // Liberar memoria antes de volver a importar la escena (grid y solver no se estan borrando)
        release();

        if (importScene(name))
        {
            // Inicializar el simulador de nuevo tras importar la escena
            init();
        } 
        else ERROR("Simulation not reset because it is not possible to import scene");
    }
    else
    {
        ERROR("There is no file path to reimport the scene");
    }
}

void SPHSimulation::release()
{
    for (uint i = 0; i < numberFluidModels(); ++i)
    {
        delete fluidObjects[i];
    }

    for (uint i = 0; i < numberBoundaryModels(); ++i)
    {
        delete boundaryObjects[i];
    }

    fluidObjects.clear();
    boundaryObjects.clear();
}

void SPHSimulation::init()
{
    initKernels();
    initGrid();
    initMasses();
    solver->init();
    setTime(0);

    printInfo();
}

void SPHSimulation::initGrid()
{
    grid->reserve(2 * getTotalParticles());
}

void SPHSimulation::initKernels()
{
    Poly6::setSupportRadius(supportRadius);
    Spiky::setSupportRadius(supportRadius);
    ViscoK::setSupportRadius(supportRadius);
    Cohesion::setSupportRadius(supportRadius);
    Adhesion::setSupportRadius(supportRadius);
    CubicSpline::setSupportRadius(supportRadius);
}

void SPHSimulation::initMasses()
{
    Real diameter = static_cast<Real>(2.0) * getParticleRadius();
    Real particleVolume = pow(diameter, static_cast<Real>(3.0));

    for (uint i = 0; i < numberFluidModels(); ++i)
    {
        FluidObject* fObj = getFluidModel(i);
        Real density0 = fObj->getRefDensity();

        fObj->setMasses(density0 * particleVolume);
    }
}

bool SPHSimulation::step()
{
    Real time = getTime();
    static double compTime = 0;
    bool save = false;

    if (time >= getStartTime() && time <= getEndTime())
    {
        DEBUG("--TIME-------------- ", time, " s ------------------------------");
        DEBUG("  FRAME->", tm.getFrame());
        startCounting("Step computation ");
        solver->step();
        stopCounting("Step computation ");

        compTime += getInterval("Step computation ");
 
        DEBUG("Avg time step     -> ", time / solver->getSteps(), " s");
        DEBUG("Avg step comp     -> ", compTime / solver->getSteps(), " s");

        DEBUG("----------------------------------------------------------------");
        DEBUG();

        save = tm.hasToSave();

        if (save)
        {
            if (activeSave)
                SceneLoader::writeFluid();

            tm.setFrame(tm.getFrame() + 1);
        }
    }

    return save;
}

void SPHSimulation::run()
{
    Real time = getTime();

    while (time >= getStartTime() && time <= getEndTime())
        step();
}

void SPHSimulation::setParticleRadius(Real particleRadius)
{
    this->particleRadius = particleRadius;

    // Actualizar valor de la distancia de suavizado
    this->supportRadius = static_cast<Real>(4.0) * particleRadius;
    grid->setSmoothingLength(this->supportRadius);
}

void SPHSimulation::setSupportRadius(Real supportRadius)
{
    this->supportRadius = supportRadius;
    grid->setSmoothingLength(supportRadius);
}

void SPHSimulation::setSimulationMethod(const int method)
{
    if (method == current_method || method < 0)
        return;

    current_method = method;

    if (solver) delete solver;

    switch (method)
    {
        case WCSPH_METHOD:
            solver = new WCSPHSolver();
            break;

        case PCISPH_METHOD:
            solver = new PCISPHSolver();
            break;

        case DFSPH_METHOD:
            solver = new DFSPHSolver();
            break;
    }
}

void SPHSimulation::setBoundaryMethod(const int method)
{
    current_bh_method = method;
}

void SPHSimulation::addFluidModel(FluidObject* fObj)
{
    fluidObjects.push_back(fObj);
}

void SPHSimulation::addBoundaryModel(BoundaryObject* bObj)
{
    boundaryObjects.push_back(bObj);
}

void SPHSimulation::computeNonPressureForces()
{
    uint nFluidModels = numberFluidModels();
    for (uint fObjIdx = 0; fObjIdx < nFluidModels; ++fObjIdx)
    {
        FluidObject* fObj = getFluidModel(fObjIdx);
        for (uint i = 0; i < fObj->numberNonPressureForces(); ++i)
            fObj->getNonPressureForce(i)->step();
    }
}

uint SPHSimulation::getTotalParticles()
{
    uint totalParticles = 0;

    for (uint i = 0; i < numberFluidModels(); ++i)
    {
        totalParticles += fluidObjects[i]->getSize();
    }

    for (uint i = 0; i < numberBoundaryModels(); ++i)
    {
        totalParticles += boundaryObjects[i]->getSize();
    }

    return totalParticles;
}

void SPHSimulation::emitParticles()
{
    uint nFluidModels = numberFluidModels();
    for (uint i = 0; i < nFluidModels; ++i)
    {
        getFluidModel(i)->emitParticles();
    }
}

void SPHSimulation::clearSimInfo()
{
    simInfo.boundaryData.boundaries.clear();
    simInfo.fluidData.fluids.clear();
}

bool SPHSimulation::importScene(const std::string& path)
{
    INFO("Importing scene from ", path, "...");
    name = path;

    clearSimInfo();

    if (!SceneLoader::readConfiguration(simInfo, path))
    {
        return false;
    }

    const SceneInfo & sceneData = simInfo.sceneData;
    const FluidInfo & fluidData = simInfo.fluidData;
    const BoundaryInfo & boundaryData = simInfo.boundaryData;

    // Scene info
    setTimeStep(sceneData.timeStep); 
    setFPS(sceneData.fps);
    setMinTimeStep(sceneData.minTimeStep); 
    setMaxTimeStep(sceneData.maxTimeStep);  
    setStartTime(sceneData.startTime);
    setEndTime(sceneData.endTime); 
    setSimulationMethod(sceneData.simulationMethod);
    setBoundaryMethod(sceneData.boundaryMethod);
    setGravity(sceneData.gravity);
    setParticleRadius(sceneData.particleRadius);

    if (sceneData.simulationMethod == WCSPH_METHOD)
    {
        WCSPHSolver *wcsph = static_cast<WCSPHSolver*>(getSolver());
        wcsph->setStiffness(sceneData.stiffness);
        wcsph->setGamma(sceneData.gamma);
    }
    else if (sceneData.simulationMethod == PCISPH_METHOD)
    {
        PCISPHSolver *pcisph = static_cast<PCISPHSolver*>(getSolver());
        pcisph->setMaxError(sceneData.eta);
        pcisph->setMinIterations(sceneData.minIterations);
        pcisph->setMaxIterations(sceneData.maxIterations);
    }
    else if (sceneData.simulationMethod == DFSPH_METHOD)
    {
        DFSPHSolver *dfsph = static_cast<DFSPHSolver*>(getSolver());
        dfsph->setMaxError(sceneData.eta);
        dfsph->setMaxErrorV(sceneData.etaV);
        dfsph->setCFLFactor(sceneData.cflFactor);
        dfsph->setMinIterations(sceneData.minIterations);
        dfsph->setMaxIterations(sceneData.maxIterations);
        dfsph->setMinIterationsV(sceneData.minIterationsV);
        dfsph->setMaxIterationsV(sceneData.maxIterationsV);
    }

    // Fluid info
    uint numFluidModels = static_cast<uint>(fluidData.fluids.size());
    const std::vector<Fluid> & fluids = fluidData.fluids; // vector de fluidModels

    setViscosityMethod(fluidData.viscosityMethod);
    setSurfaceTensionMethod(fluidData.surfaceTensionMethod);
    setAdhesionMethod(fluidData.adhesionMethod);

    for (uint i = 0; i < numFluidModels; ++i)
    {
        FluidObject* fObj = new FluidObject();
        
        uint numBlocks = static_cast<uint>(fluids[i].fluidBlocks.size());
        uint numEmitters = static_cast<uint>(fluids[i].emitters.size());
        //uint numGeometries = fluids[i].geometries.size();
        
        if (numBlocks > 0)
            fObj->build(fluids[i].fluidBlocks, getParticleRadius());

        // geomettry

        // emitters
        for (uint j = 0; j < numEmitters; ++j)
        {
            fObj->addEmitter(fluids[i].emitters[j].type,
                             fluids[i].emitters[j].numParticles,
                             fluids[i].emitters[j].r,
                             fluids[i].emitters[j].v,
                             fluids[i].emitters[j].rot,
                             fluids[i].emitters[j].startTime,
                             fluids[i].emitters[j].width,
                             fluids[i].emitters[j].height,
                             fluids[i].emitters[j].spacing);

        }

        fObj->setRefDensity(fluids[i].density0);

        if (fluidData.viscosityMethod > -1 && fluids[i].viscosity != 0.0)
            fObj->setViscosityForce(fluids[i].viscosity, fluids[i].boundaryViscosity);

        if (fluidData.surfaceTensionMethod > -1 && fluids[i].surfaceTension != 0.0)
            fObj->setSurfaceTensionForce(fluids[i].surfaceTension);

        if (fluidData.adhesionMethod > -1 && fluids[i].adhesion != 0.0)
            fObj->setAdhesionForce(fluids[i].adhesion);

        addFluidModel(fObj);
    }

    // Boundary info
    uint numBoundaries = static_cast<uint>(boundaryData.boundaries.size());
    const std::vector<Boundary> & boundaries = boundaryData.boundaries;

    if (sceneData.boundaryMethod == AKINCI_BOUNDARY_METHOD)
    {
        for (uint i = 0; i < numBoundaries; ++i)
        {
            AkinciBoundaryObject* bObj = new AkinciBoundaryObject();

            uint numBox = static_cast<uint>(boundaries[i].box.size());
            uint numSphere = static_cast<uint>(boundaries[i].sphere.size());
            uint numGeometry = static_cast<uint>(boundaries[i].geometry.size());

            for (uint j = 0; j < numBox; ++j)
                bObj->addCube(boundaries[i].box[j].first.min, boundaries[i].box[j].first.max);

            for (uint j = 0; j < numSphere; ++j)
                bObj->addSphere(boundaries[i].sphere[j].first.pos, boundaries[i].sphere[j].first.radius);

            for (uint j = 0; j < numGeometry; ++j)
                bObj->addGeometry(boundaries[i].geometry[j].path, boundaries[i].geometry[j].spacing * sceneData.particleRadius * static_cast<Real>(2.0));
            
            addBoundaryModel(bObj);
        }
    }
    else if (sceneData.boundaryMethod == PCISPH_BOUNDARY_METHOD)
    {
        if (numberFluidModels() == 1)
        {
            for (uint i = 0; i < numBoundaries; ++i)
            {
                PCISPHBoundaryObject* bObj = new PCISPHBoundaryObject();
                Real density0 = getFluidModel(0)->getRefDensity(); 
                Real particleVolume = pow(static_cast<Real>(2.0) * getParticleRadius(), static_cast<Real>(3.0));
                bObj->setMass(density0 * particleVolume);
                bObj->setRefDensity(density0);
                bObj->setNormalFct(boundaries[i].normalFct);
                bObj->setTangentialFct(boundaries[i].tangFct);

                uint numBox = static_cast<uint>(boundaries[i].box.size());
                uint numSphere = static_cast<uint>(boundaries[i].sphere.size());

                for (uint j = 0; j < numBox; ++j)
                    bObj->addCube(boundaries[i].box[j].first.min, boundaries[i].box[j].first.max, boundaries[i].box[j].second);

                for (uint j = 0; j < numSphere; ++j)
                    bObj->addSphere(boundaries[i].sphere[j].first.pos, boundaries[i].sphere[j].first.radius, boundaries[i].sphere[j].second);

                addBoundaryModel(bObj);
            }
        }
        else
            INFO("PCISPH BOUNDARY METHOD is not compatible with multiphase");
    }
    else if (sceneData.boundaryMethod == CUBE_BOUNDARY_METHOD)
    {
        for (uint i = 0; i < numBoundaries; ++i)
        {
            CubeBoundaryObject* bObj = new CubeBoundaryObject();

            bObj->setNormalFct(boundaries[i].normalFct);
            bObj->setTangentialFct(boundaries[i].tangFct);

            // Este modelo solo acepta una box
            uint numBox = glm::min(static_cast<uint>(boundaries[i].box.size()), 1u);
            if (numBox > 0)
            {
                bObj->setMin(boundaries[i].box[0].first.min);
                bObj->setMax(boundaries[i].box[0].first.max);
            }

            addBoundaryModel(bObj);
        }
    }

    INFO("Scene imported!");

    return true;
}

bool SPHSimulation::importFluidState(uint frame)
{
    std::string path = name;

    std::string jsonext = ".json";
    std::string num = std::to_string(frame);
    
    uint iters = SceneLoader::MAX_DIGITS - static_cast<uint>(num.length());
    for (uint i = 0; i < iters; ++i)
        num = "0" + num;

    path.insert(path.length() - jsonext.length(), "_" + num);

    INFO("Importing fluid state from ", path, "...");

    ////////////////////////////////////////////////////////////////////
    // Importante: Por el momento se supone que solo hay un fluid object
    ////////////////////////////////////////////////////////////////////

    json config;
    std::ifstream file(path);

    if (!file.is_open())
    {
        ERROR("Cannot read ", path);
        return false;
    }

    if (numberFluidModels() == 0)
    {
        ERROR("No fluid objects in the scene");
        return false;
    }

    file >> config;

    FluidObject* fObj = getFluidModel(0);

    if (fObj->getSize() != config["numParticles"])
    {
        ERROR("Different fluid configuration scene_fluid != frame_fluid");
        return false;
    }

    #pragma omp parallel for
    for (uint i = 0; i < fObj->getSize(); ++i)
    {
        Vector3r& r = fObj->getPosition(i);
        Vector3r& v = fObj->getVelocity(i);

        r.x = config["positions"][i][0];
        r.y = config["positions"][i][1];
        r.z = config["positions"][i][2];

        v.x = config["velocities"][i][0];
        v.y = config["velocities"][i][1];
        v.z = config["velocities"][i][2];
    }

    fObj->setNumActiveParticles(config["numActiveParticles"]);
    setTime(config["time"]);
    setTimeStep(config["timeStep"]);
    tm.setFrame(static_cast<uint>(getFPS() * getTime()));

    std::cout << getFrame() << std::endl;
    exit(0);

    // Incrementar el tiempo de emision de los emisores por que si no emitira en el primer instante
    fObj->increaseNextEmitTime();

    file.close();
    return true;
}

void SPHSimulation::printInfo()
{
    std::string solverLabel = "none";
    std::string bhLabel = "none";
    std::string viscoLabel = "none";
    std::string surftenLabel = "none";
    std::string adhesionLabel = "none";

    if (current_method == WCSPH_METHOD)
        solverLabel = "Weakly Compressible SPH";
    else if (current_method == PCISPH_METHOD)
        solverLabel = "Predictive-Corrective Incompressible SPH";
    else if (current_method == DFSPH_METHOD)
        solverLabel = "Divergence-Free SPH";

    if (current_bh_method == CUBE_BOUNDARY_METHOD)
        bhLabel = "Simple box boundary handling method";
    else if (current_bh_method == PCISPH_BOUNDARY_METHOD)
        bhLabel = "PCISPH boundary handling method";
    else if (current_bh_method == AKINCI_BOUNDARY_METHOD)
        bhLabel = "Akinci boundary handling method";

    if (current_visco_method == STANDARD_VISCOSITY_METHOD)
        viscoLabel = "Standard SPH formulation of viscosity (Explicit)";
    else if (current_visco_method == ARTIFICIAL_VISCOSITY_METHOD)
        viscoLabel = "Artificial viscosity method (Explicit)";
    else if (current_visco_method == XSPH_VISCOSITY_METHOD)
        bhLabel = "XSPH viscosity method";

    if (current_surften_method == AKINCI_SURFACE_TENSION_METHOD)
        surftenLabel = "Akinci surface tension method";

    if (current_adhesion_method == AKINCI_ADHESION_METHOD)
        adhesionLabel = "Akinci adhesion method";    

    INFO("--------------------------------------------------------------------------------------");
    INFO("Launching simulation with the following configuration:");
    INFO("Number of fluid models:         ", numberFluidModels(), " fObj");
    INFO("Number of boundary models:      ", numberBoundaryModels(), " bObj");
    for (uint i = 0; i < numberFluidModels(); ++i)
        INFO("Number of particles in fObj ", i + 1,  ":    ", getFluidModel(i)->getSize(), " particles");
    for (uint i = 0; i < numberBoundaryModels(); ++i)
        INFO("Number of particles in bObj ", i + 1, ":    ", getBoundaryModel(i)->getSize(), " particles");
    INFO("Pressure solver:                " , solverLabel);
    if (current_method == WCSPH_METHOD)
    {
        WCSPHSolver *currentSolver = static_cast<WCSPHSolver*>(solver);
        INFO("Stiffness:                      ", currentSolver->getStiffness());
        INFO("Gamma:                          ", currentSolver->getGamma());
    }
    else if (current_method == PCISPH_METHOD)
    {
        PCISPHSolver *currentSolver = static_cast<PCISPHSolver*>(solver);
        INFO("Max. allowed density error:     ", currentSolver->getMaxError());
        INFO("Min. iterations:                ", currentSolver->getMinIterations());
        INFO("Max. iterations:                ", currentSolver->getMaxIterations());
    }
    else if (current_method == DFSPH_METHOD)
    {
        DFSPHSolver *currentSolver = static_cast<DFSPHSolver*>(solver);
        INFO("Max. allowed density error:     ", currentSolver->getMaxError());
        INFO("Max. allowed divergence error:  ", currentSolver->getMaxErrorV());
        INFO("Min. density iterations:        ", currentSolver->getMinIterations());
        INFO("Max. density iterations:        ", currentSolver->getMaxIterations());
        INFO("Min divergence iterations:      ", currentSolver->getMinIterationsV());
        INFO("Max divergence iterations:      ", currentSolver->getMaxIterationsV());
    }
    INFO("Boundary handling method:       ", bhLabel);
    INFO("Viscosity method:               ", viscoLabel);
    INFO("Surface tension method:         ", surftenLabel);
    INFO("Adhesion method:                ", adhesionLabel);
    INFO("Particle radius:                ", particleRadius, " m");
    INFO("Support radius:                 ", supportRadius, " m");
    INFO("Gravity:                        ", "(" , gravity.x , "," , gravity.y , "," , gravity.z , ") m/s^2");
    INFO("--------------------------------------------------------------------------------------");
    INFO();
    
    //INFO("Press any key to start the simulation");
    //getchar();
}

void SPHSimulation::updateScalarField(std::vector<std::vector<Real>>& scalarField)
{
    // SimulationSystem size is set to fluidObjects size
    // for (uint i = 0; i < getSize(); ++i)
    // {
    //     for (uint j = 0; j < fluidObjects[i]->getSize(); ++j)
    //     {
    //         scalarField[i][j] = glm::length(fluidObjects[i]->getVelocity(j));
    //     }
    // }

    for (uint i = 0; i < numberFluidModels(); ++i)
    {
        for (uint j = 0; j < fluidObjects[i]->getSize(); ++j)
        {
            scalarField[i][j] = glm::length(fluidObjects[i]->getVelocity(j));
            //scalarField[i][j] = glm::length(fluidObjects[i]->getPressure(j));
        }
    }

    for (uint i = 1; i < numberBoundaryModels(); ++i)
    {
        uint sfIdx = (i-1) + numberFluidModels();
        for (uint j = 0; j < boundaryObjects[i]->getSize(); ++j)
        {
            scalarField[sfIdx][j] = 10.0;
        }
    }
}





