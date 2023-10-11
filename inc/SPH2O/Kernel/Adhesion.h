 
#ifndef _ADHESION_KERNEL_H_
#define _ADHESION_KERNEL_H_

#include "Common/numeric_types.h"

class Adhesion
{
    protected:

        static Real valueCoef;
        static Real supportRadius;

    public:

        static Real getSupportRadius() { return supportRadius; }
        
        static void setSupportRadius(const Real sr)
        {
            supportRadius = sr;
            valueCoef = static_cast<Real>(0.007) / pow(sr, static_cast<Real>(3.25));
        }

        static Real W(const Vector3r& r)
        {
            Real mod_r = glm::length(r);

            if ((2.0 * mod_r > supportRadius) && (mod_r <= supportRadius))
            {
                Real value = - static_cast<Real>(4.0) * mod_r * mod_r / supportRadius + static_cast<Real>(6.0) * mod_r - static_cast<Real>(2.0) * supportRadius;
                return pow(value, static_cast<Real>(1.0 / 4.0));                
            }
            else
                return 0.0;
        }
};

#endif 
 
