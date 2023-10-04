#include "DFSPHSolver.h"
#include "SPHSimulation.h"
#include "Spiky.h"
#include "Poly6.h"
#include "CubicSpline.h"
#include "CubeBoundaryObject.h"
#include "PCISPHBoundaryObject.h"
#include "AkinciBoundaryObject.h"

DFSPHSolver::DFSPHSolver() : SPHSolver()
{
    minIterations = 2; 
    maxIterations = 100;
    sumIterations = 0;
    minIterationsV = 1; 
    maxIterationsV = 100;
    sumIterationsV = 0;
    cflFactor = 1.0;
    eps = 1e-5;
}

void DFSPHSolver::init()
{
    resize();
    SPHSolver::init();

    insertFluidParticles();
    neighborhoodSearch();
    computeDensities();
    computeAlpha();
}

void DFSPHSolver::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    steps ++;
    
    //sim->startCounting("npForces         ");
    sim->computeNonPressureForces();
    //sim->stopCounting("npForces         ");

    updateTimeStep();

    predictVelocities(); 

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
        for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
        {
            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

            bObj->correctVelocities();                       
        }

    //sim->startCounting("Density solver   ");
    correctDensityError();
    //sim->stopCounting("Density solver   ");

    updatePositions();

    if (sim->getBoundaryHandlingMethod() == SPHSimulation::PCISPH_BOUNDARY_METHOD)
    {
        sim->startCounting("Boundary handling");
        for (uint bObjId = 0; bObjId < sim->numberBoundaryModels(); ++bObjId)
        {
            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

            bObj->correctPositions();
        }
        sim->stopCounting("Boundary handling");
    }

    sim->emitParticles();

    //sim->startCounting("Fill grid        ");
    insertFluidParticles();
    //sim->stopCounting("Fill grid        ");

    //sim->startCounting("Neigh search     ");
    neighborhoodSearch();
    //sim->stopCounting("Neigh search     ");

    //sim->startCounting("Densities        ");
    computeDensities();
    //sim->stopCounting("Densities        ");    

    //sim->startCounting("Alpha factor     ");
    computeAlpha();
    //sim->stopCounting("Alpha factor     ");

    //sim->startCounting("Divergence solver");
    correctDivergenceError();
    //sim->stopCounting("Divergence solver");

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

    sim->setTime(sim->getTime() + sim->getTimeStep());
}

void DFSPHSolver::computeAlpha()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real supportRadius = sim->getSupportRadius();
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
            Real& alpha = getAlpha(fObjId, i);

            const Vector3i cellId = floor(ri / supportRadius);

            Vector3r sumGrad(0.0, 0.0, 0.0);
            Real sumSqGrad = 0.0;

            forall_fluid_neighbors_in_same_phase
            (
                const Vector3r& rj = fObj->getPosition(j);
                const Vector3r massGrad = fObj->getMass(j) * CubicSpline::gradW(ri - rj);

                sumGrad += massGrad;
                sumSqGrad += glm::dot(massGrad, massGrad); 
            );

            if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                    const Vector3r& rb = bObj->getPosition(b);
                    const Vector3r massGrad = bObj->getMass() * CubicSpline::gradW(ri - rb);

                    sumGrad += massGrad;
                    //sumSqGrad += glm::dot(massGrad, massGrad); // aumenta el numero de iteraciones del bucle corrector de densidad
                );
            }
            else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                    const Vector3r& rb = bObj->getPosition(b);
                    const Vector3r massGrad = density0 * bObj->getVolume(b) * CubicSpline::gradW(ri - rb);

                    sumGrad += massGrad;
                    //sumSqGrad += glm::dot(massGrad, massGrad); // aumenta el numero de iteraciones del bucle corrector de densidad
                );
            }

            Real denominator = dot(sumGrad, sumGrad) + sumSqGrad;

            if (denominator > eps)
                alpha = fObj->getDensity(i) / denominator;
            else 
                alpha = 0.0;
        }
    }
}

void DFSPHSolver::predictVelocities()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            Vector3r& v = fObj->getVelocity(i);
            Vector3r& a = fObj->getAcceleration(i);

            v += (a + sim->getGravity()) * ts;

            a = Vector3r(0.0, 0.0, 0.0); 
        }
    }
}

void DFSPHSolver::updatePositions()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            Vector3r& r = fObj->getPosition(i);
            const Vector3r& v = fObj->getVelocity(i);

            r += v * ts;
        }
    }
}

void DFSPHSolver::predictDensities()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real supportRadius = sim->getSupportRadius();
    const Real ts = sim->getTimeStep();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    uint totalNumParticles = 0;

    Real avgDensError = 0.0;
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        const Real density0 = fObj->getRefDensity();
        totalNumParticles += numParticles;

        #pragma omp parallel for reduction(+:avgDensError)
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& ri = fObj->getPosition(i);
            const Vector3r& vi = fObj->getVelocity(i);
            Real& pred_density = getPredDensity(fObjId, i);

            Real densityChange = 0;

            const Vector3i cellId = floor(ri / supportRadius);

            forall_fluid_neighbors_in_same_phase
            (
                const Vector3r& rj = fObj->getPosition(j);
                const Vector3r& vj = fObj->getVelocity(j);

                densityChange += fObj->getMass(j) * dot(vi - vj, CubicSpline::gradW(ri - rj));
            );

            if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                    Vector3r& rb = bObj->getPosition(b);
                    densityChange += bObj->getMass() * dot(vi, CubicSpline::gradW(ri - rb));
                );
            }
            else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                    Vector3r& rb = bObj->getPosition(b);
                    densityChange += density0 * bObj->getVolume(b) * dot(vi, CubicSpline::gradW(ri - rb));
                );
            }

            pred_density = fObj->getDensity(i) + densityChange * ts;
            pred_density = glm::max(pred_density, density0); // Importante que la densidad predicha nunca sea menor de 1000 ya que solo se corrige la compresión
            
            avgDensError += (pred_density - density0) / density0;
        }
    }

    if (totalNumParticles == 0)
        avgDensError = 0;
    else
        avgDensError /= totalNumParticles;

    this->avgDensError = avgDensError;

    DEBUG("Avg error         -> ", avgDensError * 100.0, "% | ", maxError * 100.0, "%");
}

// original
// void DFSPHSolver::correctDensityError()
// {
//     SPHSimulation *sim = SPHSimulation::getCurrent();
//     HashTable *grid = sim->getGrid();
//     uint numFluidObjects = sim->numberFluidModels();
//     int boundaryMethod =sim->getBoundaryHandlingMethod();
//     Real supportRadius = sim->getSupportRadius();
//     Real ts = sim->getTimeStep();
//     Real ts2 = ts * ts;

//     //densityWarmStart();

//     predictDensities();

//     iterations = 0;
//     avgDensError = 0;
//     while (((avgDensError > maxError) || (iterations < minIterations)) && iterations < maxIterations)
//     {
//         //predictDensities();

//         for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
//         {
//             FluidObject *fObj = sim->getFluidModel(fObjId);
//             uint numParticles = fObj->getNumActiveParticles();
//             Real density0 = fObj->getRefDensity();

//             #pragma omp parallel for
//             for (int i = 0; i < numParticles; ++i)
//             {
//                 Real dens_i = fObj->getDensity(i);
//                 Real predDens_i = getPredDensity(fObjId, i);
//                 Real alpha_i = getAlpha(fObjId, i);

//                 k[fObjId][i] = (predDens_i - density0) * alpha_i / (ts2 * dens_i);

//                 // residuum y s_i
//                 // s_i es el source term, lo que se quiere corregir (drho/dt o preddens - density), se calcula
//                 // aij_pj es el "density change" que se calcula en cada iteración y se añade al source term, supongo que en total sería el error de esa iteracion
//                 // residuum es max(s_i - algo, 0.0)
//             }
//         }

//         for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
//         {
//             FluidObject *fObj = sim->getFluidModel(fObjId);
//             uint numParticles = fObj->getNumActiveParticles();
//             Real density0 = fObj->getRefDensity();

//             #pragma omp parallel for
//             for (int i = 0; i < numParticles; ++i)
//             {
//                 Vector3r & ri = fObj->getPosition(i);
//                 Vector3r & vi = fObj->getVelocity(i);
//                 Real ki = k[fObjId][i];

//                 Vector3r sum(0.0, 0.0, 0.0);
                
//                 Vector3i cellId = floor(ri / supportRadius);

//                 forall_fluid_neighbors_in_same_phase
//                 (
//                     Vector3r & rj = fObj->getPosition(j);
//                     Real kj = k[fObjId][j];

//                     Real kSum = (ki + kj);

//                     if (fabs(kSum) > eps) 
//                         sum += fObj->getMass(j) * kSum * CubicSpline::gradW(ri - rj);
//                 );

//                 if (fabs(ki) > eps)
//                 {
//                     if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
//                     {
//                         forall_boundary_neighbors
//                         (
//                             PCISPHBoundaryObject *bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

//                             Vector3r & rb = bObj->getPosition(b);

//                             sum += bObj->getMass() * ki * CubicSpline::gradW(ri - rb);
//                         );
//                     } 
//                     else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
//                     {
//                         forall_boundary_neighbors
//                         (
//                             AkinciBoundaryObject *bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(bObjId));

//                             Vector3r & rb = bObj->getPosition(b);

//                             sum += density0 * bObj->getVolume(b) * ki * CubicSpline::gradW(ri - rb);
//                         );
//                     }
//                 }
                            
//                 vi -= ts * sum;
//             } 
//         }

//         predictDensities();

//         ++iterations;
//     }
//     sumIterations += iterations;

//     DEBUG("Solver its       ->", iterations);
//     DEBUG("Solver avg its   ->", sumIterations / (Real) steps);

//     this->avgIterationsTest = sumIterations / (Real) steps;
// }

void DFSPHSolver::correctDensityError()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const int boundaryMethod =sim->getBoundaryHandlingMethod();
    const Real supportRadius = sim->getSupportRadius();
    const Real ts = sim->getTimeStep();
    const Real ts2 = ts * ts;

    if (warmStart)
    {
        densityWarmStart();
    }

    iterations = 0;
    avgDensError = 0;
    while (((avgDensError > maxError) || (iterations < minIterations)) && iterations < maxIterations)
    {
        predictDensities();

        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                const Real dens_i = fObj->getDensity(i);
                const Real predDens_i = getPredDensity(fObjId, i);
                const Real alpha_i = getAlpha(fObjId, i);

                k[fObjId][i] = (predDens_i - density0) * alpha_i / (ts2 * dens_i);

                // Warm start
                k_ws[fObjId][i] += k[fObjId][i];
            }
        }

        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                const Vector3r& ri = fObj->getPosition(i);
                Vector3r& vi = fObj->getVelocity(i);
                const Real ki = k[fObjId][i];

                Vector3r sum(0.0, 0.0, 0.0);
                
                const Vector3i cellId = floor(ri / supportRadius);

                forall_fluid_neighbors_in_same_phase
                (
                    Vector3r& rj = fObj->getPosition(j);
                    Real kj = k[fObjId][j];

                    Real kSum = (ki + kj);

                    if (fabs(kSum) > eps) 
                        sum += fObj->getMass(j) * kSum * CubicSpline::gradW(ri - rj);
                );

                if (fabs(ki) > eps)
                {
                    if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
                    {
                        forall_boundary_neighbors
                        (
                            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                            Vector3r& rb = bObj->getPosition(b);

                            sum += bObj->getMass() * ki * CubicSpline::gradW(ri - rb);
                        );
                    } 
                    else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
                    {
                        forall_boundary_neighbors
                        (
                            AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                            Vector3r& rb = bObj->getPosition(b);

                            sum += density0 * bObj->getVolume(b) * ki * CubicSpline::gradW(ri - rb);
                        );
                    }
                }
                            
                vi -= ts * sum;
            } 
        }

        ++iterations;
    }

    if (warmStart)
    {
        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                k_ws[fObjId][i] *= ts2;
            }
        }
    }

    sumIterations += iterations;

    DEBUG("Solver its        -> ", iterations);
    DEBUG("Solver avg its    -> ", sumIterations / (Real) steps);
}

void DFSPHSolver::computeDivergenceError()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real supportRadius = sim->getSupportRadius();
    uint totalNumParticles = 0;

    Real avgDivError = 0.0;
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        const Real density0 = fObj->getRefDensity();
        totalNumParticles += numParticles;

        #pragma omp parallel for reduction(+:avgDivError) 
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& ri = fObj->getPosition(i);
            const Vector3r& vi = fObj->getVelocity(i);
            Real& divError = getDivError(fObjId, i);

            divError = 0;
            uint numNeighbors = 0;

            const Vector3i cellId = floor(ri / supportRadius);

            forall_fluid_neighbors_in_same_phase
            (
                const Vector3r& rj = fObj->getPosition(j);
                const Vector3r& vj = fObj->getVelocity(j);

                const Vector3r grad = CubicSpline::gradW(ri - rj);
                divError += fObj->getMass(j) * dot(vi - vj, grad);

                if (length(ri - rj) <= supportRadius)
                    numNeighbors++;
            );

            if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                    const Vector3r& rb = bObj->getPosition(b);

                    const Vector3r grad = CubicSpline::gradW(ri - rb);
                    divError += bObj->getMass() * dot(vi, grad);

                    if (length(ri - rb) <= supportRadius)
                        numNeighbors++;
                );
            } 
            else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
            {
                forall_boundary_neighbors
                (
                    AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                    const Vector3r& rb = bObj->getPosition(b);

                    const Vector3r grad = CubicSpline::gradW(ri - rb);
                    divError += density0 * bObj->getVolume(b) * dot(vi, grad);

                    if (length(ri - rb) <= supportRadius)
                        numNeighbors++;
                );
            } 

            if (numNeighbors < 20)//20) // No corregir la divergencia de las particulas que tienen deficiencia de vecinos (Importante contar solo como vecinos las particulas que estan a una distancia menor o igual que h ya que la tabla hash devuelve como vecinos mas particulas a parte de estas)
                divError = 0.0;
            else
                divError = glm::max(divError, static_cast<Real>(0.0));  // solo se toma el error de divergencia positivo ya que es el error de compresion al igual que en la densidad

            avgDivError += divError / density0;
        }
    }

    avgDivError /= totalNumParticles;
    this->avgDivError = avgDivError;

    DEBUG("Avg errorV        -> ", avgDivError * sim->getTimeStep() * 100.0, "% | ", maxErrorV * 100.0, "%");  
}

// void DFSPHSolver::correctDivergenceError()
// {
//     SPHSimulation *sim = SPHSimulation::getCurrent();
//     HashTable *grid = sim->getGrid();
//     uint numFluidObjects = sim->numberFluidModels();
//     int boundaryMethod = sim->getBoundaryHandlingMethod();
//     Real supportRadius = sim->getSupportRadius();
//     Real ts = sim->getTimeStep();

//     //divergenceWarmStart(); // De momento inestabiliza la simulacion
//     computeDivergenceError();

//     // En el codigo de dfsph se divide el maxErrorV entre el timeStep
//     // porque el avgDivError es el drho/dt o sea la variacion de la densidad respecto al tiempo
//     avgDivError = 0;
//     iterationsV = 0;
//     while (((avgDivError > maxErrorV / ts) || (iterationsV < minIterationsV)) && iterationsV < maxIterations)
//     {
//         //computeDivergenceError();

//         for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
//         {
//             FluidObject *fObj = sim->getFluidModel(fObjId);
//             uint numParticles = fObj->getNumActiveParticles();
//             Real density0 = fObj->getRefDensity();

//             #pragma omp parallel for
//             for (int i = 0; i < numParticles; ++i)
//             {
//                 Real dens_i = fObj->getDensity(i);
//                 Real divError_i = getDivError(fObjId, i);
//                 Real alpha_i = getAlpha(fObjId, i);

//                 kv[fObjId][i] = divError_i * alpha_i / (ts * dens_i);
//             }
//         }

//         for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
//         {
//             FluidObject *fObj = sim->getFluidModel(fObjId);
//             uint numParticles = fObj->getNumActiveParticles();
//             Real density0 = fObj->getRefDensity();

//             #pragma omp parallel for
//             for (int i = 0; i < numParticles; ++i)
//             {
//                 Vector3r & ri = fObj->getPosition(i);
//                 Vector3r & vi = fObj->getVelocity(i);
//                 Real kiv = kv[fObjId][i];

//                 Vector3r sum(0.0, 0.0, 0.0);

//                 Vector3i cellId = floor(ri / supportRadius);

//                 forall_fluid_neighbors_in_same_phase
//                 (
//                     Vector3r & rj = fObj->getPosition(j);
//                     Real kjv = kv[fObjId][j];

//                     Real kSumV = (kiv + kjv);

//                     if (fabs(kSumV) > eps)
//                         sum += fObj->getMass(j) * kSumV * CubicSpline::gradW(ri - rj);
//                 );

//                 if (fabs(kiv) > eps)
//                 {
//                     if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
//                     {
//                         forall_boundary_neighbors
//                         (
//                             PCISPHBoundaryObject *bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(bObjId));

//                             Vector3r & rb = bObj->getPosition(b);

//                             sum += bObj->getMass() * kiv * CubicSpline::gradW(ri - rb);
//                         );
//                     } 
//                     else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
//                     {
//                         forall_boundary_neighbors
//                         (
//                             AkinciBoundaryObject *bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(bObjId));

//                             Vector3r & rb = bObj->getPosition(b);

//                             sum += density0 * bObj->getVolume(b) * kiv * CubicSpline::gradW(ri - rb);
//                         );
//                     }
//                 }

//                 vi -= ts * sum;
//             } 
//         }

//         computeDivergenceError();

//         ++iterationsV;
//     }
//     sumIterationsV += iterationsV;

//     DEBUG("SolverV its      ->", iterationsV);
//     DEBUG("SolverV avg its  ->", sumIterationsV / (Real) steps);

//     this->avgIterationsVTest = sumIterationsV / (Real) steps;
// }

void DFSPHSolver::correctDivergenceError()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real supportRadius = sim->getSupportRadius();
    const Real ts = sim->getTimeStep();

    if (warmStart)
    {
        divergenceWarmStart();
    }

    avgDivError = 0;
    iterationsV = 0;
    while (((avgDivError > maxErrorV / ts) || (iterationsV < minIterationsV)) && iterationsV < maxIterations)
    {
        computeDivergenceError();

        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                const Real dens_i = fObj->getDensity(i);
                const Real divError_i = getDivError(fObjId, i);
                const Real alpha_i = getAlpha(fObjId, i);

                kv[fObjId][i] = divError_i * alpha_i / (ts * dens_i);

                // Warm start
                kv_ws[fObjId][i] += kv[fObjId][i];
            }
        }

        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                const Vector3r& ri = fObj->getPosition(i);
                Vector3r& vi = fObj->getVelocity(i);
                const Real kiv = kv[fObjId][i];

                Vector3r sum(0.0, 0.0, 0.0);

                const Vector3i cellId = floor(ri / supportRadius);

                forall_fluid_neighbors_in_same_phase
                (
                    const Vector3r& rj = fObj->getPosition(j);
                    const Real kjv = kv[fObjId][j];

                    const Real kSumV = (kiv + kjv);

                    if (fabs(kSumV) > eps)
                        sum += fObj->getMass(j) * kSumV * CubicSpline::gradW(ri - rj);
                );

                if (fabs(kiv) > eps)
                {
                    if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
                    {
                        forall_boundary_neighbors
                        (
                            PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                            const Vector3r& rb = bObj->getPosition(b);

                            sum += bObj->getMass() * kiv * CubicSpline::gradW(ri - rb);
                        );
                    } 
                    else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
                    {
                        forall_boundary_neighbors
                        (
                            AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                            const Vector3r& rb = bObj->getPosition(b);

                            sum += density0 * bObj->getVolume(b) * kiv * CubicSpline::gradW(ri - rb);
                        );
                    }
                }

                vi -= ts * sum;
            } 
        }

        ++iterationsV;
    }

    if (warmStart)
    {
        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                kv_ws[fObjId][i] *= ts;
            }
        }
    }

    sumIterationsV += iterationsV;

    DEBUG("SolverV its       -> ", iterationsV);
    DEBUG("SolverV avg its   -> ", sumIterationsV / (Real) steps);
}

void DFSPHSolver::densityWarmStart()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real supportRadius = sim->getSupportRadius();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real ts = sim->getTimeStep();
    const Real ts2 = ts * ts;

    predictDensities();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        const Real density0 = fObj->getRefDensity();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            const Real predDens_i = getPredDensity(fObjId, i);
            if (predDens_i > density0)
            {
                // Full 
                k_ws[fObjId][i] = k_ws[fObjId][i] / ts2; // Mejora bastante la convergencia pero en ciertos momentos se escapan partículas

                // splish
                //k_ws[fObjId][i] = static_cast<Real>(0.5) * glm::min(k_ws[fObjId][i], static_cast<Real>(0.00025)) / ts2;
            }
            else
            {
                k_ws[fObjId][i] = 0.0;
            }
        }
    }

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        const Real density0 = fObj->getRefDensity();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& ri = fObj->getPosition(i);
            Vector3r& vi = fObj->getVelocity(i);
            const Real ki = k_ws[fObjId][i];

            Vector3r sum(0.0, 0.0, 0.0);
            
            const Vector3i cellId = floor(ri / supportRadius);

            forall_fluid_neighbors_in_same_phase
            (
                const Vector3r& rj = fObj->getPosition(j);
                const Real kj = k_ws[fObjId][j];

                const Real kSum = (ki + kj);

                if (fabs(kSum) > eps) 
                    sum += fObj->getMass(j) * kSum * CubicSpline::gradW(ri - rj);
            );

            if (fabs(ki) > eps)
            {
                if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
                {
                    forall_boundary_neighbors
                    (
                        PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                        const Vector3r& rb = bObj->getPosition(b);

                        sum += bObj->getMass() * ki * CubicSpline::gradW(ri - rb);
                    );
                } 
                else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
                {
                    forall_boundary_neighbors
                    (
                        AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                        const Vector3r& rb = bObj->getPosition(b);

                        sum += density0 * bObj->getVolume(b) * ki * CubicSpline::gradW(ri - rb);
                    );
                }
            }
                        
            vi -= ts * sum;
        } 
    }

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            k_ws[fObjId][i] = 0.0;
        }
    }
}

void DFSPHSolver::divergenceWarmStart()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real supportRadius = sim->getSupportRadius();
    const Real ts = sim->getTimeStep();

    computeDivergenceError();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            const Real divError_i = getDivError(fObjId, i);
            if (divError_i > 0.0)
            {
                // Full
                kv_ws[fObjId][i] = kv_ws[fObjId][i] / ts;

                // splish
                //kv_ws[fObjId][i] = static_cast<Real>(0.5) * glm::min(kv_ws[fObjId][i], static_cast<Real>(0.5)) / ts;
            }
            else
            {
                kv_ws[fObjId][i] = 0.0;
            }
        }
    }

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();
        const Real density0 = fObj->getRefDensity();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& ri = fObj->getPosition(i);
            Vector3r& vi = fObj->getVelocity(i);
            const Real kiv = kv_ws[fObjId][i];

            Vector3r sum(0.0, 0.0, 0.0);

            const Vector3i cellId = floor(ri / supportRadius);

            forall_fluid_neighbors_in_same_phase
            (
                const Vector3r& rj = fObj->getPosition(j);
                const Real kjv = kv_ws[fObjId][j];

                const Real kSumV = (kiv + kjv);

                if (fabs(kSumV) > eps)
                    sum += fObj->getMass(j) * kSumV * CubicSpline::gradW(ri - rj);
            );

            if (fabs(kiv) > eps)
            {
                if (boundaryMethod == SPHSimulation::PCISPH_BOUNDARY_METHOD)
                {
                    forall_boundary_neighbors
                    (
                        PCISPHBoundaryObject* bObj = static_cast<PCISPHBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                        const Vector3r& rb = bObj->getPosition(b);

                        sum += bObj->getMass() * kiv * CubicSpline::gradW(ri - rb);
                    );
                } 
                else if (boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
                {
                    forall_boundary_neighbors
                    (
                        AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                        const Vector3r& rb = bObj->getPosition(b);

                        sum += density0 * bObj->getVolume(b) * kiv * CubicSpline::gradW(ri - rb);
                    );
                }
            }

            vi -= ts * sum;
        } 
    }

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; ++i)
        {
            kv_ws[fObjId][i] = 0.0;
        }
    }
}

void DFSPHSolver::resize()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();

    alpha.resize(numFluidObjects);
    divError.resize(numFluidObjects);
    predDensity.resize(numFluidObjects);    

    k.resize(numFluidObjects);
    kv.resize(numFluidObjects);
    k_ws.resize(numFluidObjects);
    kv_ws.resize(numFluidObjects);

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getSize();

        alpha[fObjId].resize(numParticles);
        divError[fObjId].resize(numParticles);
        predDensity[fObjId].resize(numParticles);    
        k[fObjId].resize(numParticles);
        kv[fObjId].resize(numParticles);
        k_ws[fObjId].resize(numParticles);
        kv_ws[fObjId].resize(numParticles);
    }
}

void DFSPHSolver::updateTimeStep()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real ts = sim->getTimeStep();

    Real maxVel = 0.0;
    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        const uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for reduction(max:maxVel)
        for (uint i = 0; i < numParticles; ++i)
        {
            const Vector3r& vi = fObj->getVelocity(i);
            const Vector3r& ai = fObj->getAcceleration(i);
            const Vector3r acc = ai + sim->getGravity();

            const Vector3r current_vi = vi + acc * ts;

            maxVel = glm::max(maxVel, glm::length(current_vi));
        }
    }

    // Real newTs;
    // Real cflTs = cflFactor * 0.4 * (2.0 * sim->getParticleRadius()) / maxVel; 

    // //newTs = cflTs;

    // if (cflTs < ts)
    //     newTs = cflTs;
    // else if (iterations > 3)
    //     newTs = ts * 0.99;
    // else 
    //     newTs = ts * 1.001;
        
    // newTs = glm::max(newTs, sim->getMinTimeStep()); 
    // newTs = glm::min(newTs, sim->getMaxTimeStep()); 

    // sim->setTimeStep(newTs);
    
    // DEBUG("Current time step->", newTs, " s (", cflTs, " s)");

    ///////////////////////////////////////////////

    // Avoid division by zero
	if (maxVel < 1.0e-9) maxVel = 1.0e-9;

    // Guardo el ts actual
    Real h = sim->getTimeStep();

    // Calculo el nuevo segun CFL
    Real cflTs = cflFactor * static_cast<Real>(0.4) * (static_cast<Real>(2.0) * sim->getParticleRadius()) / maxVel;
    sim->setTimeStep(cflTs);

    uint its = glm::max(iterations, iterationsV);

    // Ajusto el ts anterior en base a iteraciones
    if (its > 6) // original 10
        h *= static_cast<Real>(0.9); // original 0.9
    else if (its < 4) // original 5
        h *= static_cast<Real>(1.05); // original 1.1

    // Me quedo con el mínimo entre el ts anterior ajustado con iters y el CFL
    h = glm::min(h, sim->getTimeStep());
    // Ajusto entre el minimo y el maximo
    h = glm::max(h, sim->getMinTimeStep()); 
    h = glm::min(h, sim->getMaxTimeStep()); 
    sim->setTimeStep(h);

    DEBUG("Current time step -> ", h, " s (", cflTs, " s)");
}


