#ifndef _FLUID_OBJECT_H_
#define _FLUID_OBJECT_H_

#include <vector>

#include "Common/numeric_types.h"
#include "Common/SimulationObject.h"
#include "Emitter.h"
#include "SPH2O/SPHStructs.h"

class NonPressureForce;

class FluidObject : public SimulationObject
{
    protected:

        Real density_0;
        Real volume;
        std::vector<Real> mass;
        std::vector<Real> density;
        std::vector<Real> pressure;
        std::vector<Vector3r> v;
        std::vector<Vector3r> a;
        std::vector<Vector3r> n; 

        uint numActiveParticles;

        std::vector<NonPressureForce*> npForces;
        std::vector<Emitter> emitters;

    public:

        FluidObject();
        ~FluidObject();

        void build(const std::vector<BlockInfo>& fluidBlocks, const Real radius);
        void buildSphere(const Real radius);
        
        void resize(const uint) override;
        void clear() override;

        void setMasses(const Real);
        void setRefDensity(const Real density_0) { this->density_0 = density_0; }
        void setVolume(const Real volume) { this->volume = volume; }
        void setMass(const uint i, const Real mass) { this->mass[i] = mass; }
        void setDensity(const uint i, const Real density) { this->density[i] = density; }
        void setPressure(const uint i, const Real pressure) { this->pressure[i] = pressure; }
        void setNumActiveParticles(const uint n) { if (n <= getSize()) { numActiveParticles = n; } }
        void setViscosityForce(const Real, const Real);
        void setSurfaceTensionForce(const Real);
        void setAdhesionForce(const Real);
        uint numberNonPressureForces() { return static_cast<uint>(npForces.size()); }

        Real& getRefDensity() { return density_0; }
        Real& getVolume() { return volume; }
        Real& getMass(const uint i) { return mass[i]; }
        Real& getDensity(const uint i) { return density[i]; }
        Real& getPressure(const uint i) { return pressure[i]; }
        Vector3r& getVelocity(const uint i) { return v[i]; }
        Vector3r& getAcceleration(const uint i) { return a[i]; }
        Vector3r& getNormal(const uint i) { return n[i]; }
        uint getNumActiveParticles() { return numActiveParticles; } // TODO: eliminar este método y sus llamadas y reemplazar por getActiveCount
        uint getActiveCount() override { return numActiveParticles; } 
        NonPressureForce* getNonPressureForce(const uint i) { return npForces[i]; }
        
        void addEmitter(const uint type, const uint numParticles, const Vector3r& r, const Real v, const Quat4r& rot, const Real startTime, const Real w, const Real h, const Real s);
        void emitParticles();
        void increaseNextEmitTime();
};

#endif
