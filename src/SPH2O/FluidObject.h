#ifndef _FLUID_MODEL_H_
#define _FLUID_MODEL_H_

#include <vector>
#include "../Common/numeric_types.h"
#include "Emitter.h"
#include "../Common/SimulationObject.h"
#include "SPHStructs.h"

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

        void build(const std::vector<BlockInfo> & fluidBlocks, Real radius);
        void buildSphere(Real radius);
        
        void resize(const uint) override;
        void clear() override;

        void setMasses(const Real);
        void setRefDensity(Real density_0) { this->density_0 = density_0; }
        void setVolume(Real volume) { this->volume = volume; }
        void setMass(const uint i, const Real mass) { this->mass[i] = mass; }
        void setDensity(const uint i, const Real density) { this->density[i] = density; }
        void setPressure(const uint i, const Real pressure) { this->pressure[i] = pressure; }
        void setNumActiveParticles(const uint n) { if (n <= getSize()) { numActiveParticles = n; } }
        void setViscosityForce(Real, Real);
        void setSurfaceTensionForce(Real);
        void setAdhesionForce(Real);
        uint numberNonPressureForces() { return static_cast<uint>(npForces.size()); }

        Real& getRefDensity() { return density_0; }
        Real& getVolume() { return volume; }
        Real& getMass(const uint i) { return mass[i]; }
        Real& getDensity(const uint i) { return density[i]; }
        Real& getPressure(const uint i) { return pressure[i]; }
        Vector3r& getVelocity(const uint i) { return v[i]; }
        Vector3r& getAcceleration(const uint i) { return a[i]; }
        Vector3r& getNormal(const uint i) { return n[i]; }
        uint getNumActiveParticles() { return numActiveParticles; } // to do: eliminar este método y sus llamadas y reemplazar por getActiveCount
        uint getActiveCount() override { return numActiveParticles; } 
        NonPressureForce* getNonPressureForce(const uint i) { return npForces[i]; }
        
        void addEmitter(uint type, uint numParticles, Vector3r r, Real v, Quat4r rot, Real startTime, Real w, Real h, Real s);
        void emitParticles();
        void increaseNextEmitTime();
};

#endif
