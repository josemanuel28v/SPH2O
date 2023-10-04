#pragma once
#include <vector>
#include "numeric_types.h"

class SimulationObject 
{
public:

    SimulationObject() {}

    virtual uint getSize() { return static_cast<uint>(r.size()); };
    virtual uint getActiveCount() { return static_cast<uint>(r.size()); }
    virtual Vector4r* getPositions() { return r.data(); }
    virtual Vector3r& getPosition(uint i) { return reinterpret_cast<Vector3r&>(r[i]); }

    virtual void resize(const uint size)  { r.resize(size); }
    virtual void clear() { r.clear(); }

protected:

    std::vector<Vector4r> r;
};