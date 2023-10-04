#include "FluidObject.h"
#include "NonPressureForce.h"
#include "SPHSimulation.h"
#include "StandardViscosityForce.h"
#include "ArtificialViscosity.h"
#include "XSPHViscosity.h"
#include "SurfaceTensionAkinci.h"
#include "AdhesionForce.h"
#include <iostream>

FluidObject::FluidObject()
{
    // Inicializar variables y punteros que no se usen al principio a nullptr
    density_0 = 1000;

    numActiveParticles = 0;
}

FluidObject::~FluidObject()
{
    // Borrar contenido de punteros
    for (uint i = 0; i < npForces.size(); ++i)
        delete npForces[i];
}

void FluidObject::setMasses(const Real mass)
{
    #pragma omp parallel for
    for (uint i = 0; i < getSize(); ++i)
        setMass(i, mass);
}

void FluidObject::build(const std::vector<BlockInfo> & fluidBlocks, Real radius)
{
    const Real dist = static_cast<Real>(2.0) * radius;
    std::vector<Vector3u> ppedge(fluidBlocks.size());

    uint numParticles = 0;
    for (uint i = 0; i < fluidBlocks.size(); ++i)
    {
        ppedge[i] = floor((fluidBlocks[i].max - fluidBlocks[i].min) / dist + static_cast<Real>(1e-5));
        numParticles += ppedge[i].x * ppedge[i].y * ppedge[i].z;
    }

    resize(numParticles);

    uint id = 0;
    for (uint bid = 0; bid < ppedge.size(); ++bid)
    {
        for (uint i = 0; i < ppedge[bid].x; ++i)
            for (uint j = 0; j < ppedge[bid].y; ++j)
                for (uint k = 0; k < ppedge[bid].z; ++k)
                {
                    Vector3r pos(i, j, k);
                    pos *= dist;
                    pos += fluidBlocks[bid].min + radius;

                    r[id] = Vector4r(pos, 1.0);
                    ++id;
                }
    }

    numActiveParticles = getSize();
}

void FluidObject::buildSphere(Real radius)
{
    Real dist = static_cast<Real>(2.0) * radius;

    // for each sphere in fluidSpheres
    const Vector3r origin = Vector3r(0);
    const Real sphereRad = 0.5;
    Vector3r minBBox(- 2.0 * sphereRad);
    Vector3r maxBBox(2.0 * sphereRad);

    minBBox += origin;
    maxBBox += origin;

    Vector3r ppedge = floor((maxBBox - minBBox) / dist + static_cast<Real>(1e-5));
    r.clear(); // probablemente haya
    for (uint i = 0; i < ppedge.x; ++i)
    {
        for (uint j = 0; j < ppedge.y; ++j)
        {
            for (uint k = 0; k < ppedge.z; ++k)
            {
                Vector3r pos(i, j, k);
                pos *= dist;
                pos += minBBox;

                if (glm::length(origin - pos) <= sphereRad)
                {
                    r.push_back(Vector4r(pos, 1.0));
                }
            }
        }
    }

    resize(static_cast<uint>(r.size())); // comprobar si al hacer esto los valores del vector r se modifican

    numActiveParticles = getSize();
}

void FluidObject::resize(const uint size)
{
    SimulationObject::resize(size);
    mass.resize(size);
    density.resize(size);
    pressure.resize(size);
    v.resize(size);
    a.resize(size);
    n.resize(size);
}

void FluidObject::clear()
{
    SimulationObject::clear();
    mass.clear();
    density.clear();
    pressure.clear();
    v.clear();
    a.clear();
    n.clear();
}

void FluidObject::setViscosityForce(Real viscosity, Real bViscosity)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const int current_method = sim->getViscosityMethod();

    if (current_method == SPHSimulation::STANDARD_VISCOSITY_METHOD)
    {
        StandardViscosityForce *svf = new StandardViscosityForce(this);
        svf->init(viscosity, bViscosity);
        npForces.push_back(svf);
    }
    else if (current_method == SPHSimulation::ARTIFICIAL_VISCOSITY_METHOD)
    {
        ArtificialViscosity *av = new ArtificialViscosity(this);
        av->init(viscosity, bViscosity);
        npForces.push_back(av);
    }
    else if (current_method == SPHSimulation::XSPH_VISCOSITY_METHOD)
    {
        XSPHViscosity *xv = new XSPHViscosity(this);
        xv->init(viscosity, bViscosity);
        npForces.push_back(xv);
    }
}

void FluidObject::setSurfaceTensionForce(Real stCoef)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const int current_method = sim->getSurfaceTensionMethod();

    if (current_method == SPHSimulation::AKINCI_SURFACE_TENSION_METHOD)
    {
        SurfaceTensionAkinci *sta = new SurfaceTensionAkinci(this);
        sta->init(stCoef);
        npForces.push_back(sta);
    }
}

void FluidObject::setAdhesionForce(Real beta)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const int current_method = sim->getAdhesionMethod();

    if (current_method == SPHSimulation::AKINCI_ADHESION_METHOD)
    {
        AdhesionForce *ad = new AdhesionForce(this);
        ad->init(beta);
        npForces.push_back(ad);
    }
}

void FluidObject::addEmitter(uint type, uint numParticles, Vector3r r, Real v, Quat4r rot, Real startTime, Real w, Real h, Real spacing)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    SPHSolver* solver = sim->getSolver();
    HashTable* grid = sim->getGrid();

    Emitter emitter(this, type, numParticles, r, v, rot, startTime, w, h, spacing);

    // Cada vez que se añade un emisor se modifica el numero total de particulas de fluidModel y se redimensiona el solver y las nonpresureforces
    resize(getSize() + numParticles);

    for (uint i = 0; i < npForces.size(); ++i)
        npForces[i]->resize(getSize());

    emitters.push_back(emitter);
}

void FluidObject::emitParticles()
{
    for (uint i = 0; i < emitters.size(); ++i)
        emitters[i].emitParticles();
}

void FluidObject::increaseNextEmitTime()
{
    for (uint i = 0; i < emitters.size(); ++i)
        emitters[i].increaseNextTimeEmit();
}
