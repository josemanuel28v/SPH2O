#ifndef _ARTIFICIAL_VISCOSITY_FORCE_H_
#define _ARTIFICIAL_VISCOSITY_FORCE_H_

#include "SPH2O/SPHSimulation.h"
#include "NonPressureForce.h"
#include "Common/numeric_types.h"

class ArtificialViscosity: public NonPressureForce
{
    protected:

        Real viscosity;
        Real boundaryViscosity;

    public:

        ArtificialViscosity(FluidObject* fObj) : NonPressureForce(fObj) {}
        ~ArtificialViscosity() {}
        void step() override;
        void resize(const uint size) override {}
        void setViscosityCoef(const Real viscosity) { this->viscosity = viscosity; }
        void setBoundaryViscosityCoef(const Real boundaryViscosity) { this->boundaryViscosity = boundaryViscosity; }        
        Real getViscosityCoef() { return viscosity; }
        Real getBoundaryViscosityCoef() { return boundaryViscosity; }
};

#endif 
