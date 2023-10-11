#include "GUI/ObjModel.h"
#include "GUI/GLTexture.h"
#include "GUI/FactoryEngine.h"
#include "OBJLoader/OBJLoader.h"

ObjModel::ObjModel(const std::string& path)
{
	Vector3r scale = Vector3r(1.0, 1.0, 1.0);
    Vector3r translate = Vector3r(0.0, 0.0, 0.0);
    Vector3r rotate = Vector3r(M_PI * 0.5, 0.0, 0.0);

	Utilities::OBJLoader::Vec3f scale_ = {(float) scale.x, (float) scale.y, (float) scale.z};
	std::vector<Utilities::OBJLoader::Vec3f> x;
	std::vector<Utilities::MeshFaceIndices> f;
	std::vector<Utilities::OBJLoader::Vec3f> n;
	std::vector<Utilities::OBJLoader::Vec2f> tc;

	Utilities::OBJLoader::loadObj(path, &x, &f, &n, &tc, scale_);

	std::cout << x.size() << " " << n.size() << " " << tc.size() << std::endl;
}

void ObjModel::step(double deltaTime)
{}

