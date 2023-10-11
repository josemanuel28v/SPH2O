#ifndef _ADHESION_FORCE_H_
#define _ADHESION_FORCE_H_

#include "SPH2O/SPHSimulation.h"
#include "NonPressureForce.h"
#include "Common/numeric_types.h"

class AdhesionForce: public NonPressureForce
{
    protected:

        Real beta;

    public:

        AdhesionForce(FluidObject* fObj) :NonPressureForce(fObj) { resize(fObj->getSize()); }
        ~AdhesionForce() {}
        void step() override;
        void resize(const uint size) override {}
        void setAdhesionCoef(const Real beta) { this->beta = beta; }
        Real getAdhesionCoef() { return beta; }
};

#endif 