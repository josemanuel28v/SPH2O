#ifndef _BOUNDARY_MODEL_H_
#define _BOUNDARY_MODEL_H_

#include <vector>
#include "../Common/numeric_types.h"
#include "../Common/SimulationObject.h"

class BoundaryObject : public SimulationObject
{
    protected:
        
        // Contendra los rigid body que podran ser estaticos animados o dinamicos

    public:

        BoundaryObject() {}
        virtual ~BoundaryObject() {}

        virtual void resize(const uint);
        virtual void clear();
};

#endif
 
