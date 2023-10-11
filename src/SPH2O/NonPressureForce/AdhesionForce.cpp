#include "SPH2O/NonPressureForce/AdhesionForce.h"
#include "SPH2O/Kernel/Adhesion.h"
#include "SPH2O/BoundaryObject/AkinciBoundaryObject.h"

void AdhesionForce::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numParticles = fObj->getNumActiveParticles();
    const Real density0 = fObj->getRefDensity();
    const Real supportRadius = sim->getSupportRadius();

    #pragma omp parallel for 
    for (uint i = 0; i < numParticles; i++)
    {
        const Vector3r& ri = fObj->getPosition(i);
        Vector3r& ai = fObj->getAcceleration(i);
        const Vector3i cellId = glm::floor(ri / supportRadius);

        forall_boundary_neighbors
        (
            AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
            const Vector3r& rb = bObj->getPosition(b);
            const Vector3r rib = ri - rb;
            const Real mod_rib = length(rib);

            if (mod_rib > 0.0)
            {
                ai -= beta * density0 * bObj->getVolume(b) * Adhesion::W(rib) * rib / mod_rib;
            }
        );
    }
}