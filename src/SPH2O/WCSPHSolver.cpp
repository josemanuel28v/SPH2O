#include "WCSPHSolver.h"
#include "SPHSimulation.h"
#include "CubeBoundaryObject.h"
#include "PCISPHBoundaryObject.h"
#include "AkinciBoundaryObject.h"
#include "Spiky.h"
#include "CubicSpline.h"

void WCSPHSolver::init()
{
    SPHSolver::init();
}

void WCSPHSolver::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    uint numFluidObjects = sim->numberFluidModels();
    ++steps;

    //sim->startCounting("Fill grid        ");
    insertFluidParticles();
    //sim->stopCounting("Fill grid        ");

    //sim->startCounting("Neigh search     ");
    neighborhoodSearch();
    //sim->stopCounting("Neigh search     ");

    //sim->startCounting("Densities        ");
    computeDensities();
    //sim->stopCounting("Densities        ");

    //sim->startCounting("Pressures        ");
    computePressures();
    //sim->stopCounting("Pressures        ");

    //sim->startCounting("Pressure force   ");
    for (uint i = 0; i < numFluidObjects; ++i)
        computePressureForce(i);
    //sim->stopCounting("Pressure force   ");

    //sim->startCounting("NPForces         ");
    sim->computeNonPressureForces();
    //sim->stopCounting("NPForces         ");
    
    //updateTimeStep();

    //sim->startCounting("Integratio       ");
    integrate();
    //sim->stopCounting("Integration      ");

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::CUBE_BOUNDARY_METHOD)
    {
        sim->startCounting("Boundary Handling");
        for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
        {
            CubeBoundaryObject* bObj = static_cast<CubeBoundaryObject*>(sim->getBoundaryModel(bObjId));

            bObj->correctPositionAndVelocity();
        }
        sim->startCounting("Boundary Handling");
    }
    else if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        sim->startCounting("Boundary Handling");
        for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
        {
            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

            bObj->correctPositions();
            bObj->correctVelocities();
        }
        sim->stopCounting("Boundary Handling");
    }

    sim->emitParticles();

    sim->setTime(sim->getTime() + sim->getTimeStep());
}

void WCSPHSolver::computeFluidPressures(const uint fObjId)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    FluidObject* fObj = sim->getFluidModel(fObjId);
    const uint numParticles = fObj->getNumActiveParticles();
    const Real density0 = fObj->getRefDensity();

    #pragma omp parallel for
    for (uint i = 0; i < numParticles; ++i)
    {
        Real& pressure = fObj->getPressure(i);
        const Real& density = fObj->getDensity(i);

        pressure = glm::max(static_cast<Real>(0.0), stiffness * density0 / gamma * (glm::pow(density / density0, gamma) - 1));
    }
}

void WCSPHSolver::computeBoundaryPressures(const uint bObjId)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));
        const uint numParticles = bObj->getSize();
        const Real density0 = bObj->getRefDensity();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            Real& pressure = bObj->getPressure(i);
            const Real& density = bObj->getDensity(i);

            pressure = glm::max(static_cast<Real>(0.0), stiffness * density0 / gamma * (glm::pow(density / density0, gamma) - 1));
        }
    }
}

void WCSPHSolver::computePressures()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const uint numBoundaryObjects = sim->numberBoundaryModels();

    for (uint i = 0; i < numFluidObjects; ++i)
        computeFluidPressures(i);

    for (uint i = 0; i < numBoundaryObjects; ++i)
        computeBoundaryPressures(i);
}


void WCSPHSolver::computePressureForce(const uint fObjId)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    FluidObject* fObj = sim->getFluidModel(fObjId);
    HashTable* grid = sim->getGrid();
    const uint numParticles = fObj->getNumActiveParticles();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real density0 = fObj->getRefDensity();

    #pragma omp parallel for
    for (uint i = 0; i < numParticles; ++i)
    {
        const Vector3r& ri = fObj->getPosition(i);
        Vector3r& ai = fObj->getAcceleration(i);
        const Real& press_i = fObj->getPressure(i);
        const Real& dens_i = fObj->getDensity(i);

        const Vector3i cellId = floor(ri / sim->getSupportRadius());

        const Real densPress_i = press_i / (dens_i * dens_i);

        forall_fluid_neighbors_in_same_phase
        (
            const Vector3r& rj = fObj->getPosition(j);
            const Real& press_j = fObj->getPressure(j);
            const Real& dens_j = fObj->getDensity(j);

            ai -= fObj->getMass(j) * (densPress_i + press_j / (dens_j * dens_j)) * CubicSpline::gradW(ri - rj);
        );

        if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
        {
            forall_boundary_neighbors
            (
                PCISPHBoundaryObject* nbObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                const Vector3r& rb = nbObj->getPosition(b);
                const Real& press_b = nbObj->getPressure(b);
                const Real& dens_b = nbObj->getDensity(b);

                ai -= nbObj->getMass() * (densPress_i + press_b / (dens_b * dens_b)) * CubicSpline::gradW(ri - rb);     
            );
        }
        else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
        {
            const Real densPress_b = press_i / (density0 * density0); // Utilizando dens_i hay penetracion de particulas mientras que con density0 no pasa
            forall_boundary_neighbors
            (
                AkinciBoundaryObject* nbObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                const Vector3r& rb = nbObj->getPosition(b);

                ai -= density0 * nbObj->getVolume(b) * (densPress_i + densPress_b) * CubicSpline::gradW(ri - rb); 
            );
        }
    }
}

void WCSPHSolver::updateTimeStep()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    maxVel = 0.0;
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& vi = fObj->getVelocity(i);
            const Vector3r& ai = fObj->getAcceleration(i);
            const Vector3r acc = ai + sim->getGravity();

            const Vector3r current_vi = vi + acc * ts;

            maxVel = glm::max(maxVel, length(current_vi));
        }
    }

    Real cflFactor = 0.5;
    Real newTs = cflFactor * static_cast<Real>(0.4) * static_cast<Real>(2.0) * sim->getParticleRadius() / maxVel;

    if (newTs > ts)
        newTs = ts * static_cast<Real>(1.005);
    /*else if (newTs < ts)
        newTs = ;*/
        
    newTs = glm::max(newTs, sim->getMinTimeStep()); 
    newTs = glm::min(newTs, sim->getMaxTimeStep()); 

    sim->setTimeStep(newTs);
    
}
