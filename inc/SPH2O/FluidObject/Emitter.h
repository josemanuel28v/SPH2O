#ifndef _EMITTER_H_
#define _EMITTER_H_

#include <vector>

#include "Common/numeric_types.h"

class FluidObject;

class Emitter
{
    protected:

        uint type;                   // Circulo o cuadrado
        uint numParticles;           // Numero de particulas totales que se van a emitir
        std::vector<Vector3r> group; // Grupo de particulas que se emite cada vez

        Vector3r r;
        Real v; 
        Quat4r rot;

        Real startTime;              // Tiempo en el que se empieza a emitir
        Real nextTimeEmit;           // Siguiente tiempo en el que se emite

        Real width;                  // Si el tipo es circular el radio sera width
        Real height;

        Real spacing;

        FluidObject* fObj;

    public:

        inline static uint SQUARE_EMITTER = 0;
        inline static uint CIRCLE_EMITTER = 1;

        Emitter(FluidObject* fObj, const uint type, const uint numParticles, const Vector3r& r, const Real v, const Quat4r& rot, const Real startTime, const Real w, const Real h, const Real s);

        void buildCircleGroup();
        void buildSquareGroup();

        void emitParticles(); 
        void emitGroup(Real);

        void setStartTime(const Real startTime) { this->startTime = startTime; }
        void setRotation(const Vector3r& rot) { this->rot = rot; }
        void setPosition(const Vector3r& r) { this->r = r; }
        void setSize(const Real w, const Real h) { width = w; height = h; }

        void increaseNextTimeEmit();
};

#endif 
