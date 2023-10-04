#ifndef _SPIKY_H_
#define _SPIKY_H_

#include "../Common/numeric_types.h"

#include <iostream>

class Spiky
{
    private:

        static Real supportRadius;
        static Real supportRadius2;
        static Real supportRadius6;
        static Real valueCoef;
        static Real gradCoef;

    public:

        static void setSupportRadius(Real sr)
        {
            supportRadius = sr;
            supportRadius2 = sr * sr;
            supportRadius6 = sr * sr * sr * sr * sr * sr;
            valueCoef = static_cast<Real>(15.0) / (static_cast<Real>(M_PI) * supportRadius6);
            gradCoef = static_cast<Real>(45.0) / (static_cast<Real>(M_PI) * supportRadius6);
        }

        static Real W(const Vector3r& r)
        {
            Real mod_r = length(r);

            if (mod_r <= supportRadius)
            {
                Real diff = supportRadius - mod_r;

                return valueCoef * diff * diff * diff;
            }

            return 0.0;
        }

        static Vector3r gradW(const Vector3r& r)    
        {
            Real mod_r = glm::length(r);

            if (mod_r != 0 && mod_r <= supportRadius)
            {   
                Real diff = supportRadius - mod_r;
                
                return - gradCoef * diff * diff / mod_r * r;
            }

            return Vector3r(0.0, 0.0, 0.0);
        }

        static Real getSupportRadius() { return supportRadius; }
};

#endif 