#include "CubeBoundaryObject.h"
#include "SPHSimulation.h"

CubeBoundaryObject::CubeBoundaryObject() : BoundaryObject()
{
    normalFct = 0;
    tangentialFct = 1;
}

void CubeBoundaryObject::correctPositionAndVelocity()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    uint numFluidObjects = sim->numberFluidModels();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        uint numParticles = fObj->getNumActiveParticles();

        #pragma omp parallel for
        for (uint i = 0; i < numParticles; i++)
        {
            Vector3r& pos = fObj->getPosition(i);
            Vector3r& vel = fObj->getVelocity(i);

            // Coordenada X
            if (pos.x < min.x)
            {
                pos.x = min.x;
                if (vel.x < 0.0)
                    vel.x *= - normalFct;

                vel.y *= tangentialFct;
                vel.z *= tangentialFct;
            }
            else if (pos.x > max.x)
            {
                pos.x = max.x;
                if (vel.x > 0.0)
                    vel.x *= - normalFct;

                vel.y *= tangentialFct;
                vel.z *= tangentialFct;
            }

            // Coordenada Y
            if (pos.y < min.y)
            {
                pos.y = min.y;
                if (vel.y < 0.0)
                    vel.y *= - normalFct;

                vel.x *= tangentialFct;
                vel.z *= tangentialFct;
            }
            else if (pos.y > max.y)
            {
                pos.y = max.y;
                if (vel.y > 0.0)
                    vel.y *= - normalFct;

                vel.x *= tangentialFct;
                vel.z *= tangentialFct;
            }

            // Coordenada Z
            if (pos.z < min.z)
            {
                pos.z = min.z;
                if (vel.z < 0.0)
                    vel.z *= - normalFct;

                vel.x *= tangentialFct;
                vel.y *= tangentialFct;
            }
            else if (pos.z > max.z)
            {
                pos.z = max.z;
                if (vel.z > 0.0)
                    vel.z *= - normalFct;

                vel.x *= tangentialFct;
                vel.y *= tangentialFct;
                
            }
        }
    }
} 
