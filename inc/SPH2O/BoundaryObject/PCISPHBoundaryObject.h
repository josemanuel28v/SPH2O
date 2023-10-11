#ifndef _PCISPH_BOUNDARY_OBJECT_H_
#define _PCISPH_BOUNDARY_OBJECT_H_

#include "BoundaryObject.h"

class PCISPHBoundaryObject: public BoundaryObject
{
    protected:

        std::vector<Vector3r> v;
        std::vector<Vector3r> n;
        std::vector<Real> density;
        std::vector<Real> pressure;

        Real density0; 
        Real mass;

        Real normalFct;
        Real tangentialFct;

    public:

        void setNormalFct(const Real normalFct) { this->normalFct = normalFct; }
        void setTangentialFct(const Real tangentialFct) { this->tangentialFct = tangentialFct; }
        void setRefDensity(const Real density0) { this->density0 = density0; }
        void setMass(const Real mass) { this->mass = mass; }

        Real getNormalFct() { return normalFct; }
        Real getTangentialFct() { return tangentialFct; }
        Real getRefDensity() { return density0; }
        Real getMass() { return mass; }
        
        Real& getDensity(const uint i) { return density[i]; }
        Real& getPressure(const uint i) { return pressure[i]; }

        void sampleCube(const Vector3r& min, const Vector3r& max, std::vector<Vector3r>& points, std::vector<Vector3r>& normals);
        void sampleSphere(const Vector3r& origen, const Real radius, std::vector<Vector3r>& points);

        void addCube(const Vector3r& min, const Vector3r& max, const bool inv);
        void addSphere(const Vector3r& origen, const Real radius, const bool inv);

        void resize(const uint) override;
        void clear() override;

        void correctVelocities();
        void correctPositions();
        void correctPredPositions();
};

#endif  
