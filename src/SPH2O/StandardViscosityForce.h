#ifndef _STANDARD_VISCOSITY_FORCE_H_
#define _STANDARD_VISCOSITY_FORCE_H_

#include "SPHSimulation.h"
#include "NonPressureForce.h"
#include "AkinciBoundaryObject.h"
#include "ViscoK.h"
#include "../Common/numeric_types.h"

#include <iostream>

class StandardViscosityForce: public NonPressureForce
{
    protected:

        Real viscosity;
        Real boundaryViscosity;

    public:

        StandardViscosityForce(FluidObject* fObj) :NonPressureForce(fObj) {}

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
            const Real density0 = fObj->getRefDensity();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; ++i)
            {
                const Vector3r& ri = fObj->getPosition(i);
                Vector3r& vi = fObj->getVelocity(i);
                Vector3r& ai = fObj->getAcceleration(i);

                Vector3r viscoForce(0.0);

                const Vector3i cellId = floor(ri / sim->getSupportRadius());

                forall_fluid_neighbors_in_same_phase
                (
                    Vector3r& rj = fObj->getPosition(j);
                    Vector3r& vj = fObj->getVelocity(j);
                    viscoForce += fObj->getMass(j) * (vj - vi) / fObj->getDensity(j) * ViscoK::laplW(ri - rj);
                );

                const Real kinematicVisco = viscosity / fObj->getRefDensity(); 
                viscoForce *= kinematicVisco;

                ai += viscoForce;

                viscoForce = Vector3r(0.0, 0.0, 0.0);

                if (boundaryViscosity != 0.0 && boundaryMethod == SPHSimulation::AKINCI_BOUNDARY_METHOD)
                {
                    forall_boundary_neighbors
                    (
                        AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));

                        Vector3r& rb = bObj->getPosition(b);

                        viscoForce += density0 * bObj->getVolume(b) * (- vi) / fObj->getDensity(i) * ViscoK::laplW(ri - rb);
                    );

                    const Real kinematicVisco = boundaryViscosity / fObj->getRefDensity(); 
                    viscoForce *= kinematicVisco;

                    ai += viscoForce;
                }
            }
        }

        void resize(const uint ) override {}
        const Real getViscosity() { return viscosity; }
        Real getBoundaryViscosity() { return boundaryViscosity; }
};

#endif 

 
