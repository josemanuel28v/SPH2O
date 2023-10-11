#ifndef _STANDARD_VISCOSITY_FORCE_H_
#define _STANDARD_VISCOSITY_FORCE_H_

#include "SPH2O/SPHSimulation.h"
#include "NonPressureForce.h"
#include "Common/numeric_types.h"

class StandardViscosityForce: public NonPressureForce
{
    protected:

        Real viscosity;
        Real boundaryViscosity;

    public:

        StandardViscosityForce(FluidObject* fObj) : NonPressureForce(fObj) {}
        ~StandardViscosityForce() {}
        void step() override;
        void resize(const uint size) override {}
        void setViscosityCoef(const Real viscosity) { this->viscosity = viscosity; }
        void setBoundaryViscosityCoef(const Real boundaryViscosity) { this->boundaryViscosity = boundaryViscosity; }
        Real getViscosityCoef() { return viscosity; }
        Real getBoundaryViscosityCoef() { return boundaryViscosity; }
};

#endif 

 
