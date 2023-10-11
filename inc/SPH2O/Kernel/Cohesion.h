#ifndef _COHESION_KERNEL_H_
#define _COHESION_KERNEL_H_

#include "Common/numeric_types.h"

class Cohesion
{
    protected:

        static Real supportRadius;
        static Real supportRadius6;
        static Real supportRadius9;
        static Real valueCoef1;
        static Real valueCoef2;

    public:

        static Real getSupportRadius() { return supportRadius; }
        
        static void setSupportRadius(const Real sr)
        {
            supportRadius = sr;
            supportRadius6 = sr * sr * sr * sr * sr * sr;
            supportRadius9 = sr * sr * sr * sr * sr * sr * sr * sr * sr;
            valueCoef1 = static_cast<Real>(32.0 / M_PI) / supportRadius9;
            valueCoef2 = supportRadius6 / static_cast<Real>(64.0);
        }

        static Real W(const Vector3r& r)
        {
            Real value = 0;
            const Real mod_r = glm::length(r);

            if ((2.0 * mod_r > supportRadius) && (mod_r <= supportRadius))
                value = valueCoef1 * pow(supportRadius - mod_r, static_cast<Real>(3.0)) * pow(mod_r, static_cast<Real>(3.0));
            else if ((mod_r > 0) && (2.0 * mod_r <= supportRadius))
                value = valueCoef1 * static_cast<Real>(2.0) * pow(supportRadius - mod_r, static_cast<Real>(3.0)) * pow(mod_r, static_cast<Real>(3.0)) - valueCoef2;

            return value;
        }
};	

#endif 
 
