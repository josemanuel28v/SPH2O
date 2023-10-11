#ifndef _SURFACE_TENSION_FORCE_AKINCI_H_
#define _SURFACE_TENSION_FORCE_AKINCI_H_

#include <vector>

#include "SPH2O/SPHSimulation.h"
#include "NonPressureForce.h"
#include "Common/numeric_types.h"

class SurfaceTensionAkinci: public NonPressureForce
{
    protected:

        Real stCoef;
        std::vector<Vector3r> normal;

    public:

        SurfaceTensionAkinci(FluidObject* fObj) : NonPressureForce(fObj) { resize(fObj->getSize()); }
        ~SurfaceTensionAkinci() {}
        void step() override;
        void resize(const uint size) override { normal.resize(size); }
        Vector3r& getNormal(const uint i) { return normal[i]; }
        void setSurfaceTensionCoef(const Real stCoef) { this-> stCoef = stCoef; }
        Real getSurfaceTensionCoef() { return stCoef; }
};

#endif 