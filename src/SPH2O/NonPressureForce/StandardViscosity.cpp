#include "SPH2O/NonPressureForce/StandardViscosityForce.h"
#include "SPH2O/BoundaryObject/AkinciBoundaryObject.h"
#include "SPH2O/Kernel/ViscoK.h"

void StandardViscosityForce::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numParticles = fObj->getNumActiveParticles();
    const int boundaryMethod = sim->getBoundaryHandlingMethod();
    const Real density0 = fObj->getRefDensity();

    #pragma omp parallel for
    for (uint i = 0; i < numParticles; ++i)
    {
        const Vector3r& ri = fObj->getPosition(i);
        const Vector3r& vi = fObj->getVelocity(i);
        Vector3r& ai = fObj->getAcceleration(i);

        Vector3r viscoForce(0.0);

        const Vector3i cellId = floor(ri / sim->getSupportRadius());

        forall_fluid_neighbors_in_same_phase
        (
            const Vector3r& rj = fObj->getPosition(j);
            const Vector3r& vj = fObj->getVelocity(j);

            viscoForce += fObj->getMass(j) * (vj - vi) / fObj->getDensity(j) * ViscoK::laplW(ri - rj);
        );

        viscoForce *= viscosity / fObj->getDensity(i);
        ai += viscoForce;

        viscoForce = Vector3r(0.0, 0.0, 0.0);
        if (boundaryViscosity != 0.0 && boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
        {
            forall_boundary_neighbors
            (
                AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                const Vector3r& rb = bObj->getPosition(b);

                viscoForce += density0 * bObj->getVolume(b) * (- vi) / fObj->getDensity(i) * ViscoK::laplW(ri - rb);
            );

            viscoForce *= boundaryViscosity / fObj->getDensity(i);
            ai += viscoForce;
        }
    }
}