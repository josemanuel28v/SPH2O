#pragma once
#define GLAD_ONLY_HEADERS
#include "common.h"
#include "Object3d.h"

class TrianguloRot : public Object3D
{
public:

    TrianguloRot();
    virtual void step(double deltaTime) override;
};