#include "SPH2O/NonPressureForce/SurfaceTensionAkinci.h"
#include "SPH2O/Kernel/CubicSpline.h"
#include "SPH2O/Kernel/Cohesion.h"

void SurfaceTensionAkinci::step()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numParticles = fObj->getNumActiveParticles();
    const Real density0 = fObj->getRefDensity();
    const Real supportRadius = sim->getSupportRadius();

    // Compute normals
    #pragma omp parallel for
    for (uint i = 0; i < numParticles; i++)
    {
        const Vector3r& ri = fObj->getPosition(i);
        Vector3r& ni = getNormal(i);

        ni = Vector3r(0.0, 0.0, 0.0);

        const Vector3r cellId = floor(ri / supportRadius);
        
        forall_fluid_neighbors_in_same_phase
        (
            const Vector3r& rj = fObj->getPosition(j);

            const Real vol = fObj->getMass(j) / fObj->getDensity(j);
            ni += vol * CubicSpline::gradW(ri - rj);
        );

        ni *= supportRadius;
    }

    // Compute curvature and cohesion force
    #pragma omp parallel for 
    for (uint i = 0; i < numParticles; i++)
    {
        const Vector3r& ri = fObj->getPosition(i);
        Vector3r& ai = fObj->getAcceleration(i);
        const Vector3r& ni = getNormal(i);
        const Real dens_i = fObj->getDensity(i);
        
        const Vector3r cellId = floor(ri / supportRadius);
        
        forall_fluid_neighbors_in_same_phase
        (
            Vector3r acc(0.0, 0.0, 0.0);

            const Vector3r& rj = fObj->getPosition(j);
            const Vector3r& nj = getNormal(j);
            const Real dens_j = fObj->getDensity(j);

            const Vector3r rij = ri - rj;
            const Real rijMag = glm::length(rij);
            const Real kij = static_cast<Real>(2.0) * density0 / (dens_i + dens_j);

            // Cohesion
            if (rijMag > 0.0 && rijMag <= supportRadius)
            {
                acc -= stCoef * fObj->getMass(j) * Cohesion::W(rij) * rij / rijMag;
                acc -= stCoef * (ni - nj);
            }

            // Curvature
            //acc -= stCoef * (ni - nj);

            ai += kij * acc;
        );
    }
}