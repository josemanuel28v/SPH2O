#ifndef _WCSPH_SOLVER_H_
#define _WCSPH_SOLVER_H_

#include <vector>

#include "SPHSolver.h"

class WCSPHSolver: public SPHSolver
{
    protected:

        Real stiffness;
        Real gamma;

    public:

        void init();
        void step();

        void computeFluidPressures(const uint);
        void computeBoundaryPressures(const uint);
        void computePressures();
        void computePressureForce(const uint);

        void setStiffness(const Real stiffness) { this->stiffness = stiffness; }
        void setGamma(const Real gamma) { this->gamma = gamma; }

        Real getStiffness() { return stiffness; }
        Real getGamma() { return gamma; }

        void updateTimeStep();

        void resize() {}
};

#endif
