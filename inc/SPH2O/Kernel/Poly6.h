#ifndef _POLY6_KERNEL_H
#define _POLY6_KERNEL_H_

#include "Common/numeric_types.h"

class Poly6
{
    protected:

        static Real supportRadius;
        static Real supportRadius2;
        static Real supportRadius9;
        static Real valueCoef;
        static Real gradCoef;

    public:

        static Real getSupportRadius() { return supportRadius; }
        
        static void setSupportRadius(const Real sr)
        {
            supportRadius = sr;
            supportRadius2 = sr * sr;
            supportRadius9 = sr * sr * sr * sr * sr * sr * sr * sr * sr;
            valueCoef = static_cast<Real>(315.0) / (static_cast<Real>(64.0 * M_PI) * supportRadius9);
            gradCoef = static_cast<Real>(945.0) / (static_cast<Real>(32.0 * M_PI) * supportRadius9);
        }

        static Real W(const Vector3r& r)
        {
            Real mod_r = length(r);
            Real mod_r2 = mod_r * mod_r;

            if (mod_r2 <= supportRadius2)
            {
                Real diff = supportRadius2 - mod_r2;

                return valueCoef * diff * diff * diff;
            }

            return 0.0;
        }

        static Vector3r gradW(const Vector3r& r)
        {
            Real mod_r = length(r);
            Real mod_r2 = mod_r * mod_r;

            if (mod_r2 <= supportRadius2)
            {
                Real diff = supportRadius2 - mod_r2;
                return - diff * diff * gradCoef * r;
            }

            return Vector3r(0.0);
        }
};


#endif
