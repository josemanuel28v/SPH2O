#ifndef _AKINCI_BOUNDARY_OBJECT_H_
#define _AKINCI_BOUNDARY_OBJECT_H_

#include <string>

#include "BoundaryObject.h"

class AkinciBoundaryObject: public BoundaryObject
{
    protected:

        std::vector<Vector3r> v;
        std::vector<Real> volume;

    public:

        void computeVolume();

        Real& getVolume(const uint i) { return volume[i]; }

        void sampleCube(const Vector3r& min, const Vector3r& max, std::vector<Vector3r>& points);
        void sampleSphere(const Vector3r& origen, const Real radius, std::vector<Vector3r>& points);
        void sampleGeometry(const std::string& path, const Real maxDistance, const Vector3r& scale, const Vector3r& translate, const Vector3r& rotate, std::vector<Vector3r>& points);

        void addCube(const Vector3r& min, const Vector3r& max);
        void addSphere(const Vector3r& origen, const Real radius);
        void addGeometry(const std::string& path, const Real particleRadius);

        void resize(const uint) override;
        void clear() override;
};

#endif  
 
