#ifndef _SIMULATION_SYSTEM_H_
#define _SIMULATION_SYSTEM_H_

#include <vector>

#include "SimulationObject.h"

class SimulationSystem
{
    public:
        virtual void reset() = 0;
        virtual void init() = 0;
        virtual void deferredInit() = 0;
        virtual bool step() = 0;
        virtual void release() = 0;
        virtual size_t getSize() = 0;
        virtual SimulationObject* getSimObject(const uint i) = 0;
        virtual void updateScalarField(std::vector<std::vector<Real>>& scalarField) = 0;
};

#endif