#include "SPH2O/NonPressureForce/XSPHViscosity.h"
#include "SPH2O/BoundaryObject/AkinciBoundaryObject.h"
#include "SPH2O/Kernel/Spiky.h"
#include "SPH2O/Kernel/CubicSpline.h"

void XSPHViscosity::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numParticles = fObj->getNumActiveParticles();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real supportRadius = sim->getSupportRadius();
    const Real density0 = fObj->getRefDensity();

    #pragma omp parallel for 
    for (uint i = 0; i < numParticles; i++)
    {
        const Vector3r& ri = fObj->getPosition(i);
        const Vector3r& vi = fObj->getVelocity(i);
        Vector3r& ai = fObj->getAcceleration(i);

        const Vector3i cellId = floor(ri / supportRadius);

        forall_fluid_neighbors_in_same_phase
        (
            const Vector3r& rj = fObj->getPosition(j);
            const Vector3r& vj = fObj->getVelocity(j);

            ai += viscosity * (fObj->getMass(j) / fObj->getDensity(j)) * (vj - vi) * CubicSpline::W(ri - rj) / sim->getTimeStep();
        );

        if (boundaryViscosity != 0.0 && boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
        {
            forall_boundary_neighbors
            (
                AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                const Vector3r& rb = bObj->getPosition(b);

                ai +=  boundaryViscosity * (density0 * bObj->getVolume(b) / fObj->getDensity(i)) * (- vi) * CubicSpline::W(ri - rb) / sim->getTimeStep();
            );
        }
    }
}