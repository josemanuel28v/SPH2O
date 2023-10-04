#ifndef _SPH_SOLVER_H_
#define _SPH_SOLVER_H_

#include "../Common/numeric_types.h"

class SPHSolver
{
    protected:

        uint iterations;
        uint minIterations;
        uint maxIterations;

        uint steps;
        uint sumIterations;

        Real maxError;
        Real maxVel;   

    public:

        SPHSolver();
        virtual ~SPHSolver();

        virtual void computeFluidDensities(const uint);
        virtual void computeBoundaryDensities(const uint);
        virtual void computeDensities();

        virtual void integrate();

        virtual void insertFluidParticles();
        virtual void insertBoundaryParticles();
        virtual void neighborhoodSearch();

        virtual void init() = 0;
        virtual void step() = 0;
        virtual void resize() = 0;

        void setMaxError(Real error) { maxError = error; }
        void setMinIterations(uint minIt) { minIterations = minIt; }
        void setMaxIterations(uint maxIt) { maxIterations = maxIt; }

        Real getMaxError() { return maxError; }
        Real getMaxVel() { return maxVel; }
        uint getSteps() { return steps; }
        uint getMinIterations() { return minIterations; }
        uint getMaxIterations() { return maxIterations; }
};

#endif
