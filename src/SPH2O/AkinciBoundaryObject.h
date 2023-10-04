#ifndef _AKINCI_BOUNDARY_MODEL_H_
#define _AKINCI_BOUNDARY_MODEL_H_

#include "BoundaryObject.h"
#include <string>

class AkinciBoundaryObject: public BoundaryObject
{
    protected:

        std::vector<Vector3r> v;
        std::vector<Real> volume;

    public:

        void computeVolume();

        Real& getVolume(const uint i) { return volume[i]; }

        void sampleCube(Vector3r min, Vector3r max, std::vector<Vector3r> &);
        void sampleSphere(Vector3r origen, Real radius, std::vector<Vector3r> &);
        void sampleGeometry(std::string path, Real maxDistance, Vector3r scale, Vector3r translate, Vector3r rotate, std::vector<Vector3r> &);

        void addCube(Vector3r min, Vector3r max);
        void addSphere(Vector3r origen, Real radius);
        void addGeometry(std::string, Real particleRadius);

        void resize(const uint) override;
        void clear() override;
};

#endif  
 
