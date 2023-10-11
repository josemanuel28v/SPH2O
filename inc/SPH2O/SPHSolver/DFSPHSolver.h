#ifndef _DFSPH_SOLVER_H_
#define _DFSPH_SOLVER_H_

#include <vector>

#include "SPHSolver.h"

class DFSPHSolver: public SPHSolver
{
    protected:

        std::vector<std::vector<Real>> alpha;
        std::vector<std::vector<Real>> divError;
        std::vector<std::vector<Real>> predDensity;    
        std::vector<std::vector<Real>> k;    
        std::vector<std::vector<Real>> kv; 
        std::vector<std::vector<Real>> k_ws;    
        std::vector<std::vector<Real>> kv_ws;    

        Real avgDensError;
        Real avgDivError;
        Real maxErrorV; 
        
        Real cflFactor;

        bool warmStart = true;

        uint sumIterationsV;
        uint minIterationsV;
        uint maxIterationsV;
        uint iterationsV;
        
        Real eps;

    public:

        DFSPHSolver();
        void init();
        void step();

        void computeAlpha();
        void predictVelocities();
        void updatePositions();
        void correctDensityError();
        void correctDivergenceError();

        void predictDensities();
        void computeDivergenceError();

        void divergenceWarmStart();
        void densityWarmStart();

        void setCFLFactor(const Real factor) { cflFactor = factor; }
        void setMaxErrorV(const Real etaV) { maxErrorV = etaV; }
        void setMinIterationsV(const uint minIt) { minIterationsV = minIt; }
        void setMaxIterationsV(const uint maxIt) { maxIterationsV = maxIt; }

        Real getMaxErrorV() { return maxErrorV; }
        uint getMinIterationsV() { return minIterationsV; }
        uint getMaxIterationsV() { return maxIterationsV; }

        Real& getAlpha(const uint fObjId, const uint i) { return alpha[fObjId][i]; }
        Real& getDivError(const uint fObjId, const uint i) { return divError[fObjId][i]; }
        Real& getPredDensity(const uint fObjId, const uint i) { return predDensity[fObjId][i]; }

        void updateTimeStep();

        void resize();
};

#endif  
