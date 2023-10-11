#pragma once
#include "Particle.h"
#include "Object.h"
#include "Common/SimulationSystem.h"
#include <vector>
#include <iostream>

class GraphicSimulationSystem : public Object
{
public:
    GraphicSimulationSystem(SimulationSystem* simSystem) { this->simSystem = simSystem; }

    void reset() { simSystem->reset(); }
    void init() 
    { 
        simSystem->init(); 
        scalarField.resize(getSize());
        for (uint i = 0; i < getSize(); ++i)
        {
            scalarField[i].resize(getSimObject(i)->getSize());
        }
    }
    void deferredInit() { simSystem->deferredInit(); }
    bool step() { return simSystem->step(); }
    void release() { simSystem->release(); }

    size_t getSize() { return simSystem->getSize(); }
    SimulationObject* getSimObject(uint i) { return simSystem->getSimObject(i); }

    void addPrototype(Particle* prototype) { prototypes.push_back(prototype); }
    Particle* getPrototype(uint i) { return prototypes[i]; }

    Real* getScalarField(const uint i) { return scalarField[i].data(); }
    void updateScalarField() { simSystem->updateScalarField(scalarField); }

protected:

    std::vector<std::vector<Real>> scalarField;
    std::vector<Particle*> prototypes;
    SimulationSystem* simSystem;
};