#ifndef _CUBE_BOUNDARY_OBJECT_H_
#define _CUBE_BOUNDARY_OBJECT_H_

#include "BoundaryObject.h"

class CubeBoundaryObject: public BoundaryObject
{
    protected:

        Vector3r min;
        Vector3r max;
        Real normalFct;
        Real tangentialFct;

    public:

        CubeBoundaryObject();

        void correctPositionAndVelocity();

        void setNormalFct(const Real normalFct) { this->normalFct = normalFct; }
        void setTangentialFct(const Real tangentialFct) { this->tangentialFct = tangentialFct; }
        void setMin(const Vector3r& min) { this->min = min; }
        void setMax(const Vector3r& max) { this->max = max; }

        Real getNormalFct() { return normalFct; }
        Real getTangentialFct() { return tangentialFct; }
};

#endif 
