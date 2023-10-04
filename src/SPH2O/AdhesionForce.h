 
#ifndef _ADHESION_FORCE_H_
#define _ADHESION_FORCE_H_

#include "SPHSimulation.h"
#include "NonPressureForce.h"
#include "Adhesion.h"
#include "AkinciBoundaryObject.h"
#include "../Common/numeric_types.h"
#include <vector>

class AdhesionForce: public NonPressureForce
{
    protected:

        Real beta;

    public:

        AdhesionForce(FluidObject* fObj) :NonPressureForce(fObj) { resize(fObj->getSize()); }
        ~AdhesionForce() {}

        void init(Real beta) { this->beta = beta; }
        void step() override
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

        void resize(const uint size) override {}
        Real getAdhesion() { return beta; }
};

#endif 