#include "Sphere3D.h"
#include "GLTexture.h"
#include "FactoryEngine.h"

Sphere3D::Sphere3D()
{
	Mesh3D* mesh = new Mesh3D();
	float radius = 0.5f;
	int sectorCount = 20;
	int stackCount = 20;

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = static_cast<float>(2.0 * M_PI) / sectorCount;
	float stackStep = static_cast<float>(M_PI) / stackCount;
	float sectorAngle, stackAngle;

	for(int i = 0; i <= stackCount; ++i)
	{
		stackAngle = static_cast<float>(M_PI / 2.0) - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// first and last vertices have same position and normal, but different tex coords
		for(int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			vertex_t v;

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			// vertices.push_back(x);
			// vertices.push_back(y);
			// vertices.push_back(z);
			v.position.x = x;
			v.position.y = y;
			v.position.z = z;

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			// normals.push_back(nx);
			// normals.push_back(ny);
			// normals.push_back(nz);
			v.normal.x = nx;
			v.normal.y = ny;
			v.normal.z = nz;

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			// texCoords.push_back(s);
			// texCoords.push_back(t);
			v.textCoord.x = s;
			v.textCoord.y = t;

			mesh->addVertex(v);
		}
	}

	int k1, k2;
	for(int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if(i != 0)
			{
				// indices.push_back(k1);
				// indices.push_back(k2);
				// indices.push_back(k1 + 1);

				mesh->addTriangle(k1, k2, k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if(i != (stackCount-1))
			{
				// indices.push_back(k1 + 1);
				// indices.push_back(k2);
				// indices.push_back(k2 + 1);

				mesh->addTriangle(k1 + 1, k2, k2 + 1);
			}

			// store indices for lines
			// vertical lines for all stacks, k1 => k2
			// lineIndices.push_back(k1);
			// lineIndices.push_back(k2);
			// if(i != 0)  // horizontal lines except 1st stack, k1 => k+1
			// {
			// 	lineIndices.push_back(k1);
			// 	lineIndices.push_back(k1 + 1);
			// }
		}
	}

	Material* mat = FactoryEngine::getNewMaterial();

	mat->loadPrograms({"../../data/shaders/program.vert", "../../data/shaders/program.frag"});
	mat->setShininess(50);
	mat->setTexturing(false);
	mat->setLighting(false);
	mat->setDepthWrite(true);
	mat->setCulling(false);
	mat->setWireframe(true);

	mesh->setMaterial(mat);

	this->setMesh(Mesh3D::ptr(mesh));
}

void Sphere3D::step(double deltaTime)
{}

