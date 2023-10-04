#include "SPHSolver.h" 
#include "PCISPHBoundaryObject.h"
#include "AkinciBoundaryObject.h"
#include "SPHSimulation.h"
#include "FluidObject.h"
#include "HashTable.h"
#include "Poly6.h"
#include "CubicSpline.h"
#include "Spiky.h"
#include <iostream>

SPHSolver::SPHSolver()
{
    steps = 0;
    iterations = 0;
    minIterations = 3;
    maxIterations = 100;
    sumIterations = 0;
    maxError = 0.01; // 1% density error
}

void SPHSolver::init()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();

    insertBoundaryParticles();

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::AKINCI_BOUNDARY_METHOD)
    {
        neighborhoodSearch();
        uint numBoundaryObjects = sim->numberBoundaryModels();

        for (uint bObjId = 0; bObjId < numBoundaryObjects; ++bObjId)
        {
            AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(bObjId));
            bObj->computeVolume();
        }
    }
}

SPHSolver::~SPHSolver() {}

void SPHSolver::computeFluidDensities(const uint fObjId)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    FluidObject* fObj = sim->getFluidModel(fObjId);
    const uint numParticles = fObj->getNumActiveParticles();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real density0 = fObj->getRefDensity();

    HashTable* grid = sim->getGrid();

    // Calcular densidad para las particulas de fluido de un fluid model
    #pragma omp parallel for
    for (uint i = 0; i < numParticles; ++i)
    {
        Real& density = fObj->getDensity(i);
        const Vector3r& ri = fObj->getPosition(i);

        const Vector3i cellId = floor(ri / sim->getSupportRadius());
        density = 0;

        // Contribucion de las partículas de fluido vecinas
        forall_fluid_neighbors_in_same_phase
        (
            const Vector3r& rj = fObj->getPosition(j);
            density += fObj->getMass(j) * CubicSpline::W(ri - rj);
        );

        // Contribucion de todos los boundary models
        if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
        {
            forall_boundary_neighbors
            (
                PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                const Vector3r& rb = bObj->getPosition(b);
                density += bObj->getMass() * CubicSpline::W(ri - rb);
            );
        }
        else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
        {
            forall_boundary_neighbors
            (
                AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                const Vector3r& rb = bObj->getPosition(b);
                density += density0 * bObj->getVolume(b) * CubicSpline::W(ri - rb);
            );
        }
    }
}

void SPHSolver::computeBoundaryDensities(const uint bObjId)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));
        const size_t numParticles = bObj->getSize();

        #pragma omp parallel for 
        for (int i = 0; i < numParticles; ++i)
        {
            Real& density = bObj->getDensity(i);
            const Vector3r& ri = bObj->getPosition(i);

            Vector3i cellId = floor(ri / sim->getSupportRadius());
            density = 0;

            forall_fluid_neighbors
            (
                const Vector3r& rj = nfObj->getPosition(j);
                density += nfObj->getMass(j) * CubicSpline::W(ri - rj);
            );

            forall_boundary_neighbors
            (
                PCISPHBoundaryObject* nbObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                const Vector3r& rb = nbObj->getPosition(b);
                density += nbObj->getMass() * CubicSpline::W(ri - rb);
            );
        }
    }
}

void SPHSolver::computeDensities()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const uint numBoundaryObjects = sim->numberBoundaryModels();

    for (uint i = 0; i < numFluidObjects; ++i)
        computeFluidDensities(i);

    for (uint i = 0; i < numBoundaryObjects; ++i)
        computeBoundaryDensities(i);
}

void SPHSolver::integrate()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    maxVel = 0.0;
    for (uint fObjIndex = 0; fObjIndex < numFluidObjects; ++fObjIndex)
    {
        FluidObject* fObj = sim->getFluidModel(fObjIndex);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for 
        for (uint i = 0; i < numParticles; ++i)
        {
            Vector3r& r = fObj->getPosition(i);
            Vector3r& v = fObj->getVelocity(i);
            Vector3r& a = fObj->getAcceleration(i);

            a += sim->getGravity();

            v += a * ts;
            r += v * ts;

            a = Vector3r(0, 0, 0);

            maxVel = glm::max(maxVel, length(v));
        }
    }
}

void SPHSolver::insertFluidParticles()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    FluidObject* fObj;

    // Vaciar el grid antes de insertar las particulas
    grid->clear();

    // Insertar las particulas de fluido en el grid
    for (uint fluidModelIndex = 0; fluidModelIndex < numFluidObjects; ++fluidModelIndex)
    {
        fObj = sim->getFluidModel(fluidModelIndex);
        const uint numParticles = fObj->getNumActiveParticles();

        for (uint i = 0; i < numParticles; ++i)
        {
            grid->insertFluidParticle(fObj->getPosition(i), i, fluidModelIndex); 
        }
    }
}

void SPHSolver::insertBoundaryParticles()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numBoundaryObjects = sim->numberBoundaryModels();

    // Vaciar el grid antes de insertar las particulas
    grid->clearB();

    // Insertar las particulas de fluido en el grid
    for (uint bObjId = 0; bObjId < numBoundaryObjects; ++bObjId)
    {
        BoundaryObject* bObj = sim->getBoundaryModel(bObjId); 
        const uint numParticles = bObj->getSize();

        for (uint i = 0; i < numParticles; ++i)
            grid->insertBoundaryParticle(bObj->getPosition(i), i, bObjId); 
    }
}

void SPHSolver::neighborhoodSearch()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();

    grid->neighborhoodSearch();
}



