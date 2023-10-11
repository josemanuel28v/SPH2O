#ifndef _XSPH_VISCOSITY_FORCE_H_
#define _XSPH_VISCOSITY_FORCE_H_

#include "SPH2O/SPHSimulation.h"
#include "NonPressureForce.h"
#include "Common/numeric_types.h"

class XSPHViscosity: public NonPressureForce
{
    protected:

        Real viscosity;
        Real boundaryViscosity;

    public:

        XSPHViscosity(FluidObject* fObj) : NonPressureForce(fObj) {}
        ~XSPHViscosity() {}
        void step() override;
        void resize(const uint size) override {}
        void setViscosityCoef(const Real viscosity) { this->viscosity = viscosity; }
        void setBoundaryViscosityCoef(const Real boundaryViscosity) { this->boundaryViscosity = boundaryViscosity; }
        Real getViscosityCoef() { return viscosity; }
        Real getBoundaryViscosityCoef() { return boundaryViscosity; }
};

#endif 
