#include "SPH2O/FluidObject/Emitter.h"
#include "SPH2O/SPHSimulation.h"
#include <glm/gtc/matrix_transform.hpp>

Emitter::Emitter(FluidObject* fObj, uint type, uint numParticles, const Vector3r& r, Real v, const Quat4r& rot, Real startTime, Real w, Real h, Real spacing)
{
    this->fObj = fObj;
    this->type = type;
    this->numParticles = numParticles;
    this->r = r;
    this->v = v;
    this->rot = rot;
    this->startTime = startTime;
    this->nextTimeEmit = startTime;
    this->width = w;
    this->height = h;
    this->spacing = spacing;

    if (type == CIRCLE_EMITTER)
        buildCircleGroup();
    else if (type == SQUARE_EMITTER)
        buildSquareGroup();
}

void Emitter::buildCircleGroup()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const Real particleRadius = sim->getParticleRadius();
    const Real dist = static_cast<Real>(2.0) * particleRadius;
    const uint numLevels = static_cast<uint>(floor((width - static_cast<Real>(0.5) * dist) / dist));

    group.clear();
    group.push_back(Vector3r(0.0, 0.0, 0.0));
    for (uint i = 0; i < numLevels; ++i)
    {
        const Real radius = (i + 1) * dist; // Radio en el que se colocaran las particulas
        const Real l = static_cast<Real>(2.0 * M_PI) * radius; // Longitud de la circunferencia del radio calculado
        const uint nParts = static_cast<uint>(floor (l / dist));
        
        const Real inc = static_cast<Real>(2.0 * M_PI) / nParts;
        for (uint i = 0; i < nParts; ++i)
        {
            const Real angle = i * inc;
            const Vector3r position = Vector3r(radius * sin(angle), radius * cos(angle), 0.0);

            group.push_back(position);
        }
    }
}

void Emitter::buildSquareGroup()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const Real particleRadius = sim->getParticleRadius();
    const Real dist = particleRadius * static_cast<Real>(2.0);

    const uint numX = static_cast<uint>(floor(width / dist));
    const uint numY = static_cast<uint>(height / dist);

    const Real realWidth = (numX - 1) * dist;
    const Real realHeight = (numY - 1) * dist;

    group.clear();
    for (uint i = 0; i < numX; ++i)
        for (uint j = 0; j < numY; ++j)
        {
            const Vector3r pos = Vector3r(realWidth * 0.5 - i * dist, realHeight * 0.5 - j * dist, 0.0);
            group.push_back(pos);
        }
}

void Emitter::emitParticles()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    Real time = sim->getTime();

    if (time >= nextTimeEmit)
    {
        emitGroup(time - nextTimeEmit);
    }
}

void Emitter::emitGroup(Real timeOffset)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const Real particleRadius = sim->getParticleRadius();
    const Real dist = particleRadius * static_cast<Real>(2.0);
    const uint numActiveParticles = fObj->getNumActiveParticles();

    uint id = numActiveParticles;
    if (numActiveParticles + group.size() <= numActiveParticles + numParticles) 
    {
        for (uint i = 0; i < group.size(); ++i)
        {
            Vector3r& position = fObj->getPosition(id);
            Vector3r& velocity = fObj->getVelocity(id);

            velocity = Vector3r(0, 0, v);
            position = group[i] + timeOffset * (velocity + sim->getGravity() * timeOffset);

            position = rot * Vector4r(position, 1.0);
            velocity = rot * Vector4r(velocity, 1.0);

            position += r;

            ++id;
        }

        fObj->setNumActiveParticles(numActiveParticles + static_cast<uint>(group.size()));
        numParticles -= static_cast<uint>(group.size());

        nextTimeEmit += spacing * static_cast<Real>(1.105) * dist / v; // TODO: Tener en cuenta la gravedad proyectada sobre la velocidad (si g y v tienen la misma direccion g se tiene en cuenta por completo y el siguiente tiempo de emision sera mas pronto debido a que las particulas se aceleran mas)
    }
}

// TODO: Solucionable guardando el siguiente tiempo de emision en el frame
void Emitter::increaseNextTimeEmit()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const Real particleRadius = sim->getParticleRadius();
    const Real dist = particleRadius * static_cast<Real>(2.0);
    nextTimeEmit += spacing * static_cast<Real>(1.105) * dist / v;
}