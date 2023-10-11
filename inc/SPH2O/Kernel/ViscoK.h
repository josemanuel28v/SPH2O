#ifndef _VISCO_KERNEL_H_
#define _VISCO_KERNEL_H_

#include "Common/numeric_types.h"

class ViscoK
{
    private:

        static Real supportRadius;
        static Real supportRadius2;
        static Real supportRadius3;
        static Real supportRadius6;
        static Real laplCoef;

    public:

        static Real getSupportRadius() { return supportRadius; }
        
        static void setSupportRadius(const Real sr)
        {
            supportRadius = sr;
            supportRadius2 = sr * sr;
            supportRadius3 = sr * sr * sr ;
            supportRadius6 = sr * sr * sr * sr * sr * sr;
            laplCoef = static_cast<Real>(45.0) / (static_cast<Real>(M_PI) * supportRadius6);
        }

        static Real laplW(const Vector3r& r)
        {
            Real mod_r = length(r);

            if (mod_r <= supportRadius)
                return laplCoef * (supportRadius - mod_r);

            return 0.0;
        }
};

#endif
 
