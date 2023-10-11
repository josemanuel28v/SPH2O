#ifndef _PCISPH_SOLVER_H_
#define _PCISPH_SOLVER_H_

#include <vector>

#include "SPHSolver.h"

class PCISPHSolver: public SPHSolver
{
    protected:

        std::vector<std::vector<Vector3r>> predR;
        std::vector<std::vector<Vector3r>> predV;
        std::vector<std::vector<Vector3r>> pressureAcc;
        std::vector<Real> scalingFactor;

        Real maxDensityError;
        Real avgDensityError;

    public:

        void init();
        void step();

        void computeScalingFactor();
        void pressureSolver();
        void initPressure();
        void predictVelocityAndPosition();
        void predictDensities();
        void predictFluidDensities(const uint);
        void predictBoundaryDensities(const uint);
        void updatePressure();
        void computePressureAcc();
        void integrate();

        void setPressureAcc(const uint fObjId, const uint i, const Vector3r& a) { pressureAcc[fObjId][i] = a; }
        void setPredR(const uint fObjId, const uint i, const Vector3r& r) { predR[fObjId][i] = r; }
        void setPredV(const uint fObjId, const uint i, const Vector3r& v) { predV[fObjId][i] = v; }
        void setScalingFactor(const uint fObjId, const Real sf) { scalingFactor[fObjId] = sf; }

        Vector3r& getPressureAcc(const uint fObjId, const uint i) { return pressureAcc[fObjId][i]; }
        Vector3r& getPredR(const uint fObjId, const uint i) { return predR[fObjId][i]; }
        Vector3r& getPredV(const uint fObjId, const uint i) { return predV[fObjId][i]; }
        Real& getScalingFactor(const uint fObjId) { return scalingFactor[fObjId]; }

        std::vector<std::vector<Vector3r>>* getPredR() { return &predR; }
        std::vector<std::vector<Vector3r>>* getPredV() { return &predV; }

        void resize();
};

#endif  
