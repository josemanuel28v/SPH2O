#pragma once
#include "GL1Render.h"
#include <unordered_map>

struct VAO_t
{
	uint id;   // Id del vertex buffer object
	uint v_id; // Id del buffer de vertices
	uint i_id; // Id del buffer de indices
	uint mvp_id;
	uint color_id;
	uint tan_id;
};

class GL4Render : public GL1Render
{
public:
	GL4Render(int width, int height) : GL1Render(width, height) {};
	virtual void setupObject(Object* obj) override;
	virtual void removeObject(Object* obj, bool instancing) override;
	virtual void drawObject(Object* obj) override;
	virtual void drawObjects(const std::vector<Object*>& objs) override;
	virtual void setupObject(Object* obj, unsigned numInstances) override;
	virtual void drawObject(Object* obj, unsigned numInstances, glm::vec4* mvps, Real* scalarField) override;

private:
	std::unordered_map<uint, VAO_t> bufferObjects;
};

