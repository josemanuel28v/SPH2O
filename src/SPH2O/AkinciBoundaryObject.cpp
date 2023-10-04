#include "AkinciBoundaryObject.h" 
#include "SPHSimulation.h"
#include "PCISPHSolver.h"
#include "DFSPHSolver.h"
#include "Poly6.h"
#include "CubicSpline.h"
#include "../../extern/OBJLoader/OBJLoader.h"
#include "../../extern/RegularTriangleSampling/RegularTriangleSampling.h"
#include <glm/gtc/matrix_transform.hpp>

void AkinciBoundaryObject::computeVolume()
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    HashTable* grid = sim->getGrid();
    const Real supportRadius = sim->getSupportRadius();

    #pragma omp parallel for
    for (int i = 0; i < r.size(); ++i)
    {
        const Vector3r& ri = getPosition(i);
        Real& volume = getVolume(i);
        const Vector3i cellId = glm::floor(ri / supportRadius);

        Real delta = 0.0;

        forall_boundary_neighbors
        (
            AkinciBoundaryObject* bObj = static_cast<AkinciBoundaryObject*>(sim->getBoundaryModel(nbObjId));
            const Vector3r& rb = bObj->getPosition(b);
            delta += CubicSpline::W(ri - rb);
        );

        volume = static_cast<Real>(1.0) / delta;
    }
}

void AkinciBoundaryObject::addCube(Vector3r min, Vector3r max)
{
    const uint currentBoundaryParticles = getSize();
    std::vector<Vector3r> points;

    sampleCube(min, max, points);

    resize(currentBoundaryParticles + static_cast<uint>(points.size()));

    #pragma omp parallel for
    for (size_t i = 0; i < points.size(); ++i)
    {
        size_t id = currentBoundaryParticles + i;

        r[id] = Vector4r(points[i], 1);
        v[id] = Vector3r(0.0, 0.0, 0.0);
    }
}

void AkinciBoundaryObject::addSphere(Vector3r pos, Real radius)
{
    const uint currentBoundaryParticles = getSize();
    std::vector<Vector3r> points;

    sampleSphere(pos, radius, points);

    resize(currentBoundaryParticles + static_cast<uint>(points.size()));

    #pragma omp parallel for
    for (size_t i = 0; i < points.size(); ++i)
    {
        size_t id = currentBoundaryParticles + i;

        r[id] = Vector4r(points[i], 1);
        v[id] = Vector3r(0.0, 0.0, 0.0);
    }
}

void AkinciBoundaryObject::addGeometry(std::string path, Real particleRadius)
{
    const uint currentBoundaryParticles = getSize();
    std::vector<Vector3r> points;

    const Vector3r scale = Vector3r(1.0, 1.0, 1.0);
    const Vector3r translate = Vector3r(0.0, 0.0, 0.0);
    const Vector3r rotate = Vector3r(M_PI * 0.5, 0.0, 0.0);

    sampleGeometry(path, particleRadius, scale, translate, rotate, points);

    resize(currentBoundaryParticles + static_cast<uint>(points.size()));

    #pragma omp parallel for
    for (size_t i = 0; i < points.size(); ++i)
    {
        size_t id = currentBoundaryParticles + i;

        r[id] = Vector4r(points[i], 1);
        v[id] = Vector3r(0.0, 0.0, 0.0);
    }
}

void AkinciBoundaryObject::sampleCube(Vector3r min, Vector3r max, std::vector<Vector3r> & points)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();
    const Real radius = sim->getParticleRadius();

    // Longitud del cubo en cada eje
    const Vector3r l = glm::abs(min - max);
    // Numero de particulas por eje
    const Vector3r n = glm::ceil(l / (static_cast<Real>(2.0) * radius));
    // Distancia entre particulas por cada eje
    const Vector3r d = l / n;

    uint count = 0;
    for (uint i = 0; i <= n.x; i++)
        for (uint j = 0; j <= n.y; j++)
            for (uint k = 0; k <= n.z; k++)
                if ((i == 0 || i == n.x) ||
                    (j == 0 || j == n.y) ||
                    (k == 0 || k == n.z))
                {
                    Vector3r position(i * d.x, j * d.y, k * d.z);
                    position += min;
                    points.push_back(position);

                    count++;
                }
}

void AkinciBoundaryObject::sampleSphere(Vector3r origen, Real radius, std::vector<Vector3r> & points)
{
    SPHSimulation* sim = SPHSimulation::getCurrent();

    // 1 sin solaparse, cuanto mayor sea, mas se solaparan
    const Real separacion = 1.5; 
    // Superficie de la boundary sphere / superficie de un circulo con el radio de la particula del fluido
    const uint num_parts = uint(floor(4.0 * M_PI * radius * radius / (M_PI * sim->getParticleRadius() * sim->getParticleRadius()) * separacion)); 
    const Real a = static_cast<Real>(4.0 * M_PI)  / num_parts;
    const Real d = sqrt(a);
    const uint m_theta = uint(round(M_PI / d));
    const Real d_theta = static_cast<Real>(M_PI) / m_theta;
    const Real d_phi = a / d_theta;

    for (uint m = 0; m < m_theta; ++m)
    {
        const Real theta = static_cast<Real>(M_PI * (m + 0.5)) / m_theta;
        const uint m_phi = uint(round(2.0 * M_PI * sin(theta) / d_phi));
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

void AkinciBoundaryObject::sampleGeometry(std::string path, Real maxDistance, Vector3r scale, Vector3r translate, Vector3r rotate, std::vector<Vector3r> & points)
{
    // Regular triangle sampling
	Utilities::OBJLoader::Vec3f scale_ = {(float) scale.x, (float) scale.y, (float) scale.z};
	std::vector<Utilities::OBJLoader::Vec3f> x;
	std::vector<Utilities::MeshFaceIndices> f;
	std::vector<Utilities::OBJLoader::Vec3f> n;
	std::vector<Utilities::OBJLoader::Vec2f> tc;

	Utilities::OBJLoader::loadObj(path, &x, &f, &n, &tc, scale_);

	// Cambio de los tipos de loader (Vec3f) a los tipos del sampler (Vector3r de eigen)
	std::vector<SPH::Vector3r> x_(x.size());
    #pragma omp parallel for
	for (uint i = 0; i < x.size(); ++i)
	{
		x_[i][0] = x[i][0];
		x_[i][1] = x[i][1];
		x_[i][2] = x[i][2];
	}

	std::vector<uint> f_(f.size() * 3);
    #pragma omp parallel for
	for (uint i = 0; i < f.size(); ++i)
	{
		f_[3 * i] = f[i].posIndices[0] - 1;
		f_[3 * i + 1] = f[i].posIndices[1] - 1;
		f_[3 * i + 2] = f[i].posIndices[2] - 1;
	}

	std::vector<SPH::Vector3r> samples;

	SPH::RegularTriangleSampling::sampleMesh(static_cast<uint>(x.size()), &x_[0], static_cast<uint>(f.size()), &f_[0], maxDistance, samples);

    points.resize(samples.size());

    const Vector3r axisX(1.0, 0.0, 0.0);
    const Vector3r axisY(0.0, 1.0, 0.0);
    const Vector3r axisZ(0.0, 0.0, 1.0);

    #pragma omp parallel for
	for (uint i = 0; i < points.size(); ++i)
	{
		points[i].x = samples[i][0];
		points[i].y = samples[i][1];
		points[i].z = samples[i][2];

        Vector4r tmp(points[i].x, points[i].y, points[i].z, 1.0);

        Matrix4r rotM = glm::rotate(Matrix4r(1.0), rotate.x, axisX);
        rotM = glm::rotate(rotM, rotate.y, axisY);
        rotM = glm::rotate(rotM, rotate.z, axisZ);
        
        tmp = rotM * tmp;

        points[i] = Vector3r(tmp.x, tmp.y, tmp.z);

        points[i] += translate;
	}
}


void AkinciBoundaryObject::resize(const uint size)
{
    BoundaryObject::resize(size);
    v.resize(size);
    volume.resize(size);
}

void AkinciBoundaryObject::clear()
{
    BoundaryObject::clear();
    v.clear();
    volume.clear();
}
 
