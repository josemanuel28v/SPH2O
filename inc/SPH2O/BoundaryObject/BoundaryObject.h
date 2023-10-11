#ifndef _BOUNDARY_OBJECT_H_
#define _BOUNDARY_OBJECT_H_

#include <vector>

#include "Common/numeric_types.h"
#include "Common/SimulationObject.h"

class BoundaryObject : public SimulationObject
{
    public:

        BoundaryObject() : SimulationObject() {}
        virtual ~BoundaryObject() {}
        virtual void resize(const uint size) { SimulationObject::resize(size); }
        virtual void clear() { SimulationObject::clear(); }
};

#endif
 
