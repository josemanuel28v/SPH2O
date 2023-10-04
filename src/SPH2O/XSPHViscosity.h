#ifndef _XSPH_VISCOSITY_H_
#define _XSPH_VISCOSITY_H_

#include "SPHSimulation.h"
#include "NonPressureForce.h"
#include "AkinciBoundaryObject.h"
#include "Spiky.h"
#include "CubicSpline.h"
#include "../Common/numeric_types.h"

#include <iostream>

class XSPHViscosity: public NonPressureForce
{
    protected:

        Real viscosity;
        Real boundaryViscosity;

    public:

        XSPHViscosity(FluidObject* fObj) :NonPressureForce(fObj) {}

        ~XSPHViscosity() {}

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
                        Vector3r& rb = bObj->getPosition(b);

                        ai +=  boundaryViscosity * (density0 * bObj->getVolume(b) / fObj->getDensity(i)) * (- vi) * CubicSpline::W(ri - rb) / sim->getTimeStep();
                    );
                }
            }
        }

        void resize(const uint) override {}
        Real getViscosity() { return viscosity; }
        Real getBoundaryViscosity() { return boundaryViscosity; }
};

#endif 
