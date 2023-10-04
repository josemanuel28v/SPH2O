#ifndef _ARTIFICIAL_VISCOSITY_H_
#define _ARTIFICIAL_VISCOSITY_H_

#include "SPHSimulation.h"
#include "NonPressureForce.h"
#include "AkinciBoundaryObject.h"
#include "Spiky.h"
#include "CubicSpline.h"
#include "../Common/numeric_types.h"

class ArtificialViscosity: public NonPressureForce
{
    protected:

        Real viscosity;
        Real boundaryViscosity;

    public:

        ArtificialViscosity(FluidObject* fObj) :NonPressureForce(fObj) {}
        ~ArtificialViscosity() {}

        void init(Real viscosity, Real boundaryViscosity)
        {
            this->viscosity = viscosity;
            this->boundaryViscosity = boundaryViscosity;
        }

        void step() override
        {
            SPHSimulation* sim = SPHSimulation::getCurrent();
            HashTable* grid = sim->getGrid();
            const uint numParticles = fObj->getNumActiveParticles();
            const int boundaryMethod = sim->getBoundaryHandlingMethod();
            const Real supportRadius = sim->getSupportRadius();
            const Real density0 = fObj->getRefDensity();
            const Real h2 = supportRadius * supportRadius;

            #pragma omp parallel for 
            for (uint i = 0; i < numParticles; i++)
            {
                const Vector3r& ri = fObj->getPosition(i);
                const Vector3r& vi = fObj->getVelocity(i);
                Vector3r& ai = fObj->getAcceleration(i);
                const Real dens_i = fObj->getDensity(i);

                const Vector3i cellId = floor(ri / supportRadius);

                forall_fluid_neighbors_in_same_phase
                (
                    const Vector3r& rj = fObj->getPosition(j);
                    const Vector3r& vj = fObj->getVelocity(j);

                    const Vector3r rij = ri - rj;
                    const Real dotProd = glm::dot(vi - vj, rij);
                    const Real dens_j = fObj->getDensity(j);
                    const Real sqDist = glm::dot(rij, rij);

                    ai += static_cast<Real>(10.0 * 0.1) * viscosity * (fObj->getMass(j) / dens_j) * dotProd / (sqDist + static_cast<Real>(0.01) * h2) * CubicSpline::gradW(rij);
                    //ai += static_cast<Real>(10.0 * 0.1) * viscosity * (fObj->getMass(j) / dens_j) * glm::min(dotProd, static_cast<Real>(0.0)) / (sqDist + static_cast<Real>(0.01) * h2) * CubicSpline::gradW(rirj);

                );

                if (boundaryViscosity != 0.0 && boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
                {
                    forall_boundary_neighbors
                    (
                        AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
                        const Vector3r& rb = bObj->getPosition(b);

                        const Vector3r rib = ri - rb;
                        const Real dotProd = glm::dot(vi, rib);
                        const Real sqDist = glm::dot(rib, rib);

                        ai += static_cast<Real>(10.0 * 0.1) * boundaryViscosity * (density0 * bObj->getVolume(b) / dens_i) * dotProd / (sqDist + static_cast<Real>(0.01) * h2) * CubicSpline::gradW(rib);
                        //ai += static_cast<Real>(10.0 * 0.1) * boundaryViscosity * (density0 * bObj->getVolume(b) / dens_i) * glm::min(dotProd, static_cast<Real>(0.0)) / (sqDist + static_cast<Real>(0.01) * h2) * CubicSpline::gradW(rirb);
                    );
                }


            }
        }

        void resize(const uint ) override {}
        Real getViscosity() { return viscosity; }
        Real getBoundaryViscosity() { return boundaryViscosity; }
};

#endif 
