#include "SPH2O/BoundaryObject/PCISPHBoundaryObject.h" 
#include "SPH2O/SPHSimulation.h"
#include "SPH2O/SPHSolver/PCISPHSolver.h"
#include "SPH2O/SPHSolver/DFSPHSolver.h"

void PCISPHBoundaryObject::addCube(const Vector3r& min, const Vector3r& max, const bool inverted)
{
    const uint currentBoundaryParticles = getSize();
    std::vector<Vector3r> points;
    std::vector<Vector3r> normals;

    sampleCube(min, max, points, normals);

    resize(currentBoundaryParticles + static_cast<uint>(points.size()));

    Real inv;
    inverted ? inv = 1.0 : inv = -1;

    for (uint i = 0; i < points.size(); ++i)
    {
        uint id = currentBoundaryParticles + i;

        r[id] = Vector4r(points[i], 1.0);
        n[id] = inv * normals[i];
        v[id] = Vector3r(0.0, 0.0, 0.0);
        density[id] = 0.0;
        pressure[id] = 0.0;
    }
}

void PCISPHBoundaryObject::addSphere(const Vector3r& pos, const Real radius, const bool inverted)
{
    const uint currentBoundaryParticles = getSize();
    std::vector<Vector3r> points;

    sampleSphere(pos, radius, points);

    resize(currentBoundaryParticles + static_cast<uint>(points.size()));

    Real inv;
    inverted ? inv = 1.0 : inv = -1;

    for (uint i = 0; i < points.size(); ++i)
    {
        uint id = currentBoundaryParticles + i;

        r[id] = Vector4r(points[i], 1.0);
        n[id] = inv * (pos - points[i]);
        v[id] = Vector3r(0.0, 0.0, 0.0);
        density[id] = 0.0;
        pressure[id] = 0.0;
    }
}

void PCISPHBoundaryObject::sampleCube(const Vector3r& min, const Vector3r& max, std::vector<Vector3r>& points, std::vector<Vector3r>& normals)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();

    // Longitud del cubo en cada eje
    const Real supportRadius = sim->getSupportRadius();

    const Real lx = abs(min.x - max.x);
    const Real ly = abs(min.y - max.y); 
    const Real lz = abs(min.z - max.z); 
    
    // El espaciado es 0.5 pero asi da la impresion de que las particulas botan
    const uint nx = static_cast<uint>(ceil(lx / (static_cast<Real>(0.45) * supportRadius)));
    const uint ny = static_cast<uint>(ceil(ly / (static_cast<Real>(0.45) * supportRadius)));
    const uint nz = static_cast<uint>(ceil(lz / (static_cast<Real>(0.45) * supportRadius)));
    
    // Distancia entre particulas de cada eje
    const Real dx = lx / nx; 
    const Real dy = ly / ny; 
    const Real dz = lz / nz;

    // Face normals
    const Vector3r leftNormal(1, 0, 0);
    const Vector3r rightNormal(-1, 0, 0);
    const Vector3r topNormal(0, -1, 0);
    const Vector3r bottomNormal(0, 1, 0);
    const Vector3r frontNormal(0, 0, -1);
    const Vector3r behindNormal(0, 0, 1);
    
    // Edge normals
    const Vector3r leftBottomNormal = glm::normalize((leftNormal + bottomNormal) * static_cast<Real>(0.5));
    const Vector3r behindBottomNormal = glm::normalize((behindNormal + bottomNormal) * static_cast<Real>(0.5));
    const Vector3r frontBottomNormal = glm::normalize((frontNormal + bottomNormal) * static_cast<Real>(0.5));
    const Vector3r rightBottomNormal = glm::normalize((rightNormal + bottomNormal) * static_cast<Real>(0.5));

    const Vector3r leftTopNormal = glm::normalize((leftNormal + topNormal) * static_cast<Real>(0.5));
    const Vector3r behindTopNormal = glm::normalize((behindNormal + topNormal) * static_cast<Real>(0.5));
    const Vector3r frontTopNormal = glm::normalize((frontNormal + topNormal) * static_cast<Real>(0.5));
    const Vector3r rightTopNormal = glm::normalize((rightNormal + topNormal) * static_cast<Real>(0.5));

    const Vector3r leftBehindNormal = glm::normalize((leftNormal + behindNormal) * static_cast<Real>(0.5));
    const Vector3r rightBehindNormal = glm::normalize((rightNormal + behindNormal) * static_cast<Real>(0.5));
    const Vector3r rightFrontNormal = glm::normalize((rightNormal + frontNormal) * static_cast<Real>(0.5));
    const Vector3r leftFrontNormal = glm::normalize((leftNormal + frontNormal) * static_cast<Real>(0.5));

    // Corner normals
    const Vector3r leftBehindBottomNormal = glm::normalize((leftNormal + behindNormal + bottomNormal) / static_cast<Real>(3.0));
    const Vector3r rightBehindBottomNormal = glm::normalize((rightNormal + behindNormal + bottomNormal) / static_cast<Real>(3.0));
    const Vector3r leftFrontBottomNormal = glm::normalize((leftNormal + frontNormal + bottomNormal) / static_cast<Real>(3.0));
    const Vector3r rightFrontBottomNormal = glm::normalize((rightNormal + frontNormal + bottomNormal) / static_cast<Real>(3.0));

    const Vector3r leftBehindTopNormal = glm::normalize((leftNormal + behindNormal + topNormal) / static_cast<Real>(3.0));
    const Vector3r rightBehindTopNormal = glm::normalize((rightNormal + behindNormal + topNormal) / static_cast<Real>(3.0));
    const Vector3r leftFrontTopNormal = glm::normalize((leftNormal + frontNormal + topNormal) / static_cast<Real>(3.0));
    const Vector3r rightFrontTopNormal = glm::normalize((rightNormal + frontNormal + topNormal) / static_cast<Real>(3.0));

    uint count = 0;
    for (uint i = 0; i <= nx; i++)
        for (uint j = 0; j <= ny; j++)
            for (uint k = 0; k <= nz; k++)
                if ((i == 0 || i == nx) ||
                    (j == 0 || j == ny) ||
                    (k == 0 || k == nz))
                {
                    Vector3r position(i * dx, j * dy, k * dz);
                    position += min;
                    points.push_back(position);

                    // Corners
                    if (i == 0 && j == 0 && k == 0)
                        normals.push_back(leftBehindBottomNormal);
                    else if (i == nx && j == 0 && k == 0)
                        normals.push_back(rightBehindBottomNormal);
                    else if (i == 0 && j == 0 && k == nz)
                        normals.push_back(leftFrontBottomNormal);
                    else if (i == nx && j == 0 && k == nz)
                        normals.push_back(rightFrontBottomNormal);
                    else if (i == 0 && j == ny && k == 0)
                        normals.push_back(leftBehindTopNormal);
                    else if (i == nx && j == ny && k == 0)
                        normals.push_back(rightBehindTopNormal);
                    else if (i == 0 && j == ny && k == nz)
                        normals.push_back(leftFrontTopNormal);
                    else if (i == nx && j == ny && k == nz)
                        normals.push_back(rightFrontTopNormal);

                    // Edges
                    else if (i == 0 && j == 0 && k > 0 && k < nz)
                        normals.push_back(leftBottomNormal);
                    else if (i > 0 && i < nx && j == 0 && k == 0)
                        normals.push_back(behindBottomNormal);
                    else if (i > 0 && i < nx && j == 0 && k == nz)
                        normals.push_back(frontBottomNormal);
                    else if (i == nx && j == 0 && k > 0 && k < nz)
                        normals.push_back(rightBottomNormal);

                    else if (i == 0 && j == ny && k > 0 && k < nz)
                        normals.push_back(leftTopNormal);
                    else if (i > 0 && i < nx && j == ny && k == 0)
                        normals.push_back(behindTopNormal);
                    else if (i > 0 && i < nx && j == ny && k == nz)
                        normals.push_back(frontTopNormal);
                    else if (i == nx && j == ny && k > 0 && k < nz)
                        normals.push_back(rightTopNormal);

                    else if (i == 0 && j > 0 && j < ny && k == 0)
                        normals.push_back(leftBehindNormal);
                    else if (i == nx && j > 0 && j < ny && k == 0)
                        normals.push_back(rightBehindNormal);
                    else if (i == nx && j > 0 && j < ny && k == nz)
                        normals.push_back(rightFrontNormal);
                    else if (i == 0 && j > 0 && j < ny && k == nz)
                        normals.push_back(leftFrontNormal);

                    // Faces
                    else if (i == 0 && j > 0 && j < ny && k > 0 && k < nz)
                        normals.push_back(leftNormal);
                    else if (i == nx && j > 0 && j < ny && k > 0 && k < nz)
                        normals.push_back(rightNormal);
                    else if (i > 0 && i < nx && j == ny && k > 0 && k < nz)
                        normals.push_back(topNormal);
                    else if (i > 0 && i < nx && j == 0 && k > 0 && k < nz)
                        normals.push_back(bottomNormal);
                    else if (i > 0 && i < nx && j > 0 && j < ny && k == nz)
                        normals.push_back(frontNormal);

                    else if (i > 0 && i < nx && j > 0 && j < ny && k == 0)
                        normals.push_back(behindNormal);                    

                    count++;
                }
}

void PCISPHBoundaryObject::sampleSphere(const Vector3r& origen, const Real radius, std::vector<Vector3r>& points)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();

    const Real separacion = 1.5; // 1 indica sin solaparse, cuanto mayor sea, mas se solaparan
    const uint num_parts = static_cast<uint>(floor(static_cast<Real>(4.0 * M_PI) * radius * radius / (static_cast<Real>(M_PI) * sim->getParticleRadius() * sim->getParticleRadius()) * separacion)); // Superficie de la boundary sphere / superficie de un circulo con el radio de la particula del fluido

    // Equidistant sphere
    const Real a = static_cast<Real>(4.0 * M_PI)  / num_parts;
    const Real d = sqrt(a);
    const uint m_theta = static_cast<uint>(round(M_PI / d));
    const Real d_theta = static_cast<Real>(M_PI) / m_theta;
    const Real d_phi = a / d_theta;

    for (uint m = 0; m < m_theta; ++m)
    {
        const Real theta = static_cast<Real>(M_PI * (m + 0.5)) / m_theta;
        const uint m_phi = int(round(2.0 * M_PI * sin(theta) / d_phi));
        for (uint n = 0; n < m_phi; ++n)
        {
            const Real phi = static_cast<Real>(2.0 * M_PI) * n / m_phi;
            const Real x = sin(theta) * cos(phi);
            const Real y = sin(theta) * sin(phi);
            const Real z = cos(theta);

            points.push_back(radius * Vector3r(x, y, z) + origen);
        }
    }
}

void PCISPHBoundaryObject::correctPositions()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const uint numFluidObjects = sim->numberFluidModels();
    const Real supportRadius = sim->getSupportRadius();
    const Real restRadius = static_cast<Real>(0.5) * supportRadius;

    if (sim->getSimulationMethod() == SPHSimulation::PCISPH_METHOD)
    {
        PCISPHSolver* solver = static_cast<PCISPHSolver*>(sim->getSolver());
        std::vector<std::vector<Vector3r>>* pred_r = solver->getPredR();

        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            const uint numParticles = fObj->getNumActiveParticles();

            //#pragma omp parallel for 
            for (uint i = 0; i < numParticles; i++)
            {       
                Vector3r& ri = fObj->getPosition(i); 
                Vector3r& ni = fObj->getNormal(i);
                Vector3r avg_normal(0, 0, 0);
                Real w_sum = 0;
                Real w2_sum = 0;

                Vector3i cellId = floor(ri / supportRadius);

                //#pragma omp critical asi parece que se soluciona pero ralentiza la paralelización y parece que va mas rapido sin paralelizar
                forsame_boundary_neighbors
                (
                    Vector3r& rb = getPosition(b);
                    Vector3r predict_rib = (*pred_r)[fObjId][i] - rb;

                    if (glm::length(ri - rb) < restRadius) // Colision
                    {
                        Real w = glm::max(static_cast<Real>(0.0), (restRadius - glm::length(predict_rib)) / restRadius);
                        w_sum += w;
                        w2_sum += w * (restRadius - length(predict_rib));
                        avg_normal += w * n[b];
                    } 
                );

                //#pragma omp critical

                ni = glm::normalize(avg_normal);

                //#pragma omp critical
                if (w_sum > 0)
                    ri += static_cast<Real>(1.0) / w_sum * w2_sum * ni;
            }
        }
    }
    else if (sim->getSimulationMethod() == SPHSimulation::WCSPH_METHOD || sim->getSimulationMethod() == SPHSimulation::DFSPH_METHOD)  
    {
        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            uint numParticles = fObj->getNumActiveParticles();

            //#pragma omp parallel for 
            for (uint i = 0; i < numParticles; i++)
            {       
                Vector3r& ri = fObj->getPosition(i); 
                Vector3r& ni = fObj->getNormal(i);
                Vector3r avg_normal(0, 0, 0);
                Real w_sum = 0;
                Real w2_sum = 0;

                Vector3i cellId = floor(ri / supportRadius);

                //#pragma omp critical asi parece que se soluciona pero ralentiza la paralelización y parece que va mas rapido sin paralelizar
                forsame_boundary_neighbors
                (
                    Vector3r& rb = getPosition(b);
                    Vector3r rib = ri - rb;

                    if (glm::length(rib) < restRadius) // Colision
                    {
                        Real w = glm::max(static_cast<Real>(0.0), (restRadius - length(rib)) / restRadius);
                        w_sum += w;
                        w2_sum += w * (restRadius - length(rib));
                        avg_normal += w * n[b];
                    } 
                );

                //#pragma omp critical

                ni = glm::normalize(avg_normal);

                //#pragma omp critical
                if (w_sum > 0)
                    ri += static_cast<Real>(1.0) / w_sum * w2_sum * ni;
            }
        }
    }  
}

void PCISPHBoundaryObject::correctVelocities()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    uint numFluidObjects = sim->numberFluidModels();    

    if (sim->getSimulationMethod() == SPHSimulation::PCISPH_METHOD) // con las velocidades predichas se corrige la velocidad
    {
        PCISPHSolver *solver = static_cast<PCISPHSolver*>(sim->getSolver());
        std::vector<std::vector<Vector3r>> *pred_v = solver->getPredV();

        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            uint numParticles = fObj->getNumActiveParticles();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; i++)
            {       
                Vector3r& ni = fObj->getNormal(i);
                Vector3r& vi = fObj->getVelocity(i);

                if (glm::length(ni) > 0)
                {
                    Vector3r nVel = dot((*pred_v)[fObjId][i], ni) * ni;
                    Vector3r tVel = (*pred_v)[fObjId][i] - nVel;
                    vi = tangentialFct * tVel - normalFct * nVel;
                }
            }
        }
    }
    else if (sim->getSimulationMethod() == SPHSimulation::WCSPH_METHOD)  // con la velocidad se corrige la velocidad
    {
        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            uint numParticles = fObj->getNumActiveParticles();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; i++)
            {       
                Vector3r& ni = fObj->getNormal(i);
                Vector3r& vi = fObj->getVelocity(i);

                if (glm::length(ni) > 0)
                {
                    Vector3r nVel = dot(vi, ni) * ni;
                    Vector3r tVel = vi - nVel;
                    vi = tangentialFct * tVel - normalFct * nVel;
                }
            }
        }
    }
    else if (sim->getSimulationMethod() == SPHSimulation::DFSPH_METHOD) // Con la velocidad predicha se corrige la velocidad predicha
    {
        for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
        {
            FluidObject* fObj = sim->getFluidModel(fObjId);
            uint numParticles = fObj->getNumActiveParticles();

            #pragma omp parallel for
            for (uint i = 0; i < numParticles; i++)
            {       
                Vector3r& ni = fObj->getNormal(i);
                Vector3r& vi = fObj->getVelocity(i);

                if (glm::length(ni) > 0)
                {
                    Vector3r nVel = dot(vi, ni) * ni;
                    Vector3r tVel = vi - nVel;
                    vi = tangentialFct * tVel - normalFct * nVel;
                }
            }
        }
    }
}

void PCISPHBoundaryObject::correctPredPositions()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    uint numFluidObjects = sim->numberFluidModels();
    Real supportRadius = sim->getSupportRadius();
    Real restRadius = static_cast<Real>(0.5) * supportRadius;

    PCISPHSolver *solver = static_cast<PCISPHSolver*>(sim->getSolver());
    std::vector<std::vector<Vector3r>> *pred_r = solver->getPredR();

    for (uint fObjId = 0; fObjId < numFluidObjects; ++fObjId)
    {
        FluidObject* fObj = sim->getFluidModel(fObjId);
        uint numParticles = fObj->getNumActiveParticles();

        //#pragma omp parallel for num_threads(16) 
        for (uint i = 0; i < numParticles; i++)
        {       
            Vector3r& ri = fObj->getPosition(i); 
            Vector3r& ni = fObj->getNormal(i);
            Vector3r avg_normal(0, 0, 0);
            Real w_sum = 0;
            Real w2_sum = 0;

            Vector3i cellId = floor(ri / supportRadius);

            //#pragma omp critical asi parece que se soluciona pero ralentiza la paralelización y parece que va mas rapido sin paralelizar

            forsame_boundary_neighbors
            (
                Vector3r& rb = getPosition(b);
                Vector3r& nb = n[b];
                Vector3r predict_rij = (*pred_r)[fObjId][i] - rb;

                if (glm::length(ri - rb) < restRadius) // Colision
                {
                    Real w = glm::max(static_cast<Real>(0.0), (restRadius - glm::length(predict_rij)) / restRadius);
                    w_sum += w;
                    w2_sum += w * (restRadius - length(predict_rij));
                    avg_normal += w * nb;
                } 
            );

            //#pragma omp critical

            ni = glm::normalize(avg_normal);

            //#pragma omp critical
            
            if (w_sum > 0)
                (*pred_r)[fObjId][i] += static_cast<Real>(1.0) / w_sum * w2_sum * ni;
        }
    }
}

void PCISPHBoundaryObject::resize(const uint size)
{
    BoundaryObject::resize(size);
    v.resize(size);
    n.resize(size);
    density.resize(size);
    pressure.resize(size);
}

void PCISPHBoundaryObject::clear()
{
    BoundaryObject::clear();
    v.clear();
    n.clear();
    density.clear();
    pressure.clear();
}
