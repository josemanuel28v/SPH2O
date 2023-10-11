#ifndef _NON_PRESSURE_FORCES_H_
#define _NON_PRESSURE_FORCES_H_

#include "SPH2O/FluidObject/FluidObject.h"

class NonPressureForce
{
    protected:

        FluidObject* fObj;

    public:

        NonPressureForce(FluidObject* fObj) { this->fObj = fObj; }
        virtual ~NonPressureForce() {};
        virtual void step() = 0; 
        virtual void resize(const uint) = 0;
};

#endif 

