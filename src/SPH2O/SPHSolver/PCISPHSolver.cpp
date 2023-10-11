#include "SPH2O/SPHSolver/PCISPHSolver.h"
#include "SPH2O/SPHSimulation.h"
#include "SPH2O/BoundaryObject/CubeBoundaryObject.h"
#include "SPH2O/BoundaryObject/PCISPHBoundaryObject.h"
#include "SPH2O/BoundaryObject/AkinciBoundaryObject.h"
#include "SPH2O/Kernel/Poly6.h"
#include "SPH2O/Kernel/Spiky.h"
#include "SPH2O/Kernel/CubicSpline.h"
#include "Utils/Logger.h"

void PCISPHSolver::init()
{
    //maxIterations = 10;
    resize();
    computeScalingFactor();
    
    SPHSolver::init();
}

void PCISPHSolver::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
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

    //sim->startCounting("npForces         ");
    sim->computeNonPressureForces();
    //sim->stopCounting("npForces         ");

    // Inicializar presion y fuerza de presion
    initPressure();

    // Bucle predictivo correctivo para calcular la presion 
    //sim->startCounting("Pressure solver  ");
    pressureSolver();
    //sim->stopCounting("Pressure solver  ");

    //sim->startCounting("Integration      ");
    integrate();
    //sim->stopCounting("Integration      ");

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::CUBE_BOUNDARY_METHOD)
    {
        sim->startCounting("Boundary handling");
        for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
        {
            CubeBoundaryObject* bObj = static_cast<CubeBoundaryObject*>(sim->getBoundaryModel(bObjId));

            bObj->correctPositionAndVelocity();
        }
        sim->stopCounting("Boundary handling");
    }
    else if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        sim->startCounting("Boundary handling");
        for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
        {
            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

            bObj->correctPositions();
            bObj->correctVelocities();
        }
        sim->stopCounting("Boundary handling");
    }

    sim->emitParticles();

    sim->setTime(sim->getTime() + sim->getTimeStep());
}

void PCISPHSolver::initPressure()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const uint numBoundaryObjects = sim->numberBoundaryModels();

    // Inicializar a 0 presion y aceleracion de presion de todos los fluidModels
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    { 
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            fObj->setPressure(i, 0.0);
            setPressureAcc(fObjId, i, Vector3r(0.0, 0.0, 0.0));
        }
    }

    // Inicializar a 0 la presion de las boundary particles en el metodo de bh de pcisph
    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        for (uint bObjId = 0; bObjId < numBoundaryObjects; ++bObjId)
        { 
            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));
            const uint numParticles = bObj->getSize();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                Real& pressure = bObj->getPressure(i);
                pressure = 0.0;
            }
        }
    }
}

void PCISPHSolver::predictVelocityAndPosition()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    { 
        FluidObject* fObj = sim->getFluidModel(fObjId);
        uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& r = fObj->getPosition(i);
            const Vector3r& v = fObj->getVelocity(i);
            const Vector3r& a = fObj->getAcceleration(i);
            const Vector3r& pacc = getPressureAcc(fObjId, i);

            Vector3r& predR = getPredR(fObjId, i);
            Vector3r& predV = getPredV(fObjId, i);
            Vector3r acc = a + pacc + sim->getGravity();

            predV = v + acc * ts;
            predR = r + predV * ts;
        }
    }
}

void PCISPHSolver::predictDensities()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const uint numBoundaryObjects = sim->numberBoundaryModels();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        predictFluidDensities(fObjId);

    for (uint bObjId = 0; bObjId < numBoundaryObjects; ++bObjId)
        predictBoundaryDensities(bObjId);
}

void PCISPHSolver::predictFluidDensities(const uint fObjId)
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
        const Vector3r& ri = getPredR(fObjId, i);
        const Vector3i cellId = floor(fObj->getPosition(i) / sim->getSupportRadius());

        density = 0;

        // Contribucion de las partículas de fluido vecinas
        forall_fluid_neighbors_in_same_phase
        (
            const Vector3r& rj = getPredR(fObjId, j);
            density += fObj->getMass(j) * CubicSpline::W(ri - rj);
        );

        // Contribucion de los boundaries 
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

void PCISPHSolver::predictBoundaryDensities(const uint bObjId)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));
        const uint numParticles = bObj->getSize();

        #pragma omp parallel for 
        for (uint i = 0; i < numParticles; ++i)
        {
            Real& density = bObj->getDensity(i);
            const Vector3r& ri = bObj->getPosition(i);

            const Vector3i cellId = floor(ri / sim->getSupportRadius());
            density = 0;

            forall_fluid_neighbors
            (
                const Vector3r& rj = getPredR(nfObjId, j);
                density += nfObj->getMass(j) * Poly6::W(ri - rj);
            );

            forall_boundary_neighbors
            (
                PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));
                const Vector3r& rb = bObj->getPosition(b);
                density += bObj->getMass() * Poly6::W(ri - rb);
            );
        }
    }
}

void PCISPHSolver::updatePressure()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const uint numBoundaryObjects = sim->numberBoundaryModels();

    Real avgDensityError = 0.0;
    Real maxDensityError = 0.0;
    uint totalNumParticles = 0;
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    { 
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        totalNumParticles += numParticles;
        const Real density0 = fObj->getRefDensity();

        #pragma omp parallel for reduction(+:avgDensityError) reduction(max:maxDensityError)
        for (uint i = 0; i < numParticles; ++i)
        {
            Real& pressure = fObj->getPressure(i);
    
            const Real error = glm::max(static_cast<Real>(0.0), fObj->getDensity(i) - density0);
            pressure += getScalingFactor(fObjId) * error;

            // maxError esta normalizado por lo que tambien se normaliza maxDensityError
            maxDensityError = glm::max(maxDensityError, error / density0);
            avgDensityError += error / density0;
        }
    }

    this->avgDensityError = avgDensityError / totalNumParticles;
    this->maxDensityError = maxDensityError;

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        for (uint bObjId = 0; bObjId < numBoundaryObjects; ++bObjId)
        {
            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));
            const uint numParticles = bObj->getSize();
            const Real density0 = bObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                Real& pressure = bObj->getPressure(i);
                const Real error = glm::max(static_cast<Real>(0.0), bObj->getDensity(i) - density0);

                // Como el scaling factor depende de la density0 coger un scaling factor de un fluido que tenga la misma density0
                pressure += getScalingFactor(0) * error;
            }
        }        
    }
}

void PCISPHSolver::computePressureAcc()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    { 
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        const Real density0 = fObj->getRefDensity();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& ri = fObj->getPosition(i);
            Vector3r& pacc = getPressureAcc(fObjId, i);
            const Real& press_i = fObj->getPressure(i);
            const Real& dens_i = fObj->getDensity(i);

            const Vector3i cellId = floor(ri / sim->getSupportRadius());

            pacc = Vector3r(0.0, 0.0, 0.0);

            const Real densPress_i = press_i / (dens_i * dens_i);
            forall_fluid_neighbors_in_same_phase
            (
                const Vector3r& rj = fObj->getPosition(j);
                const Real& press_j = fObj->getPressure(j);
                const Real& dens_j = fObj->getDensity(j);

                //pacc -= fObj->getMass(j) * press_j / (dens_i * dens_j) * CubicSpline::gradW(ri - rj);
                pacc -= fObj->getMass(j) * (densPress_i + press_j / (dens_j * dens_j)) * CubicSpline::gradW(ri - rj);
                //pacc -= fObj->getMass(j) * (densPress_i + press_j / (dens_j * dens_j)) * Spiky::gradW(ri - rj);
                //pacc -= fObj->getMass(j) * (press_i / (density0 * density0) + press_j / (density0 * density0)) * Spiky::gradW(ri - rj);
            );

            if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                    const Vector3r& rb = bObj->getPosition(b);
                    const Real& press_b = bObj->getPressure(b);
                    const Real& dens_b = bObj->getDensity(b);

                    pacc -= bObj->getMass() * (densPress_i + press_b / (dens_b * dens_b)) * Spiky::gradW(ri - rb);                
                );
            }
            else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
            {
                float densPress_b = press_i / (density0 * density0);
                forall_boundary_neighbors
                (
                    AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                    const Vector3r& rb = bObj->getPosition(b);

                    //pacc -= density0 * bObj->getVolume(b) * press_i / (dens_i * dens_i) * CubicSpline::gradW(ri - rb);  
                    pacc -= density0 * bObj->getVolume(b) * (densPress_i) * CubicSpline::gradW(ri - rb);                
                    //pacc -= density0 * bObj->getVolume(b) * (densPress_i) * Spiky::gradW(ri - rb);                
                );
            }
        }
    }
}

void PCISPHSolver::pressureSolver()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();

    iterations = 0;
    while ((/*maxDensityError*/ avgDensityError > maxError || iterations < minIterations) && iterations < maxIterations)
    {
        predictVelocityAndPosition();

        if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
        {
            for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
            {
                PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

                bObj->correctPredPositions();
            }
        }

        predictDensities();
        updatePressure();
        computePressureAcc();

        ++iterations;

        DEBUG("It->", iterations, "          ->", /*maxDensityError*/avgDensityError * 100.0, "%");
    }

    sumIterations += iterations;

    DEBUG("Solver its       ->", iterations);
    DEBUG("Solver avg its   ->", sumIterations / (Real) steps);
}

void PCISPHSolver::computeScalingFactor()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    const Real dist = static_cast<Real>(2.0) * sim->getParticleRadius(); 
    Vector3r sumGrad(0, 0, 0);      // Sumatorio del gradiente
    Real sumSqGrad = 0;             // Sumatorio del cuadrado del gradiente

    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            for (int k = -1; k <= 1; k++)
            {
                Vector3r rij(-i, -j, -k);
                rij *= dist;

                //Vector3r grad = Poly6::gradW(rij); 
                Vector3r grad = CubicSpline::gradW(rij);

                sumGrad += grad;
                sumSqGrad += glm::dot(grad, grad);
            }

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);

        const Real beta = static_cast<Real>(2.0) * pow(fObj->getMass(0) * ts / fObj->getRefDensity(), static_cast<Real>(2.0));
        const Real dotSumGrad = glm::dot(sumGrad, sumGrad);

        setScalingFactor(fObjId, - static_cast<Real>(1.0) / (beta * (- dotSumGrad - sumSqGrad)));
    }
}

void PCISPHSolver::resize()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();

    predR.resize(numFluidObjects);
    predV.resize(numFluidObjects);
    pressureAcc.resize(numFluidObjects);
    scalingFactor.resize(numFluidObjects);

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getSize();

        predR[fObjId].resize(numParticles);
        predV[fObjId].resize(numParticles);
        pressureAcc[fObjId].resize(numParticles);
    }
}

void PCISPHSolver::integrate()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    maxVel = 0;
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for 
        for (uint i = 0; i < numParticles; ++i)
        {
            Vector3r& r = fObj->getPosition(i);
            Vector3r& v = fObj->getVelocity(i);
            Vector3r& a = fObj->getAcceleration(i);

            a += sim->getGravity() + getPressureAcc(fObjId, i);

            v += a * ts;
            r += v * ts;

            a = Vector3r(0, 0, 0);

            maxVel = glm::max(maxVel, length(v));
        }
    }
}