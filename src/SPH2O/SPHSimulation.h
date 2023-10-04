#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include <vector>
#include <algorithm>
#include "../Common/numeric_types.h"
#include "FluidObject.h"
#include "BoundaryObject.h"
#include "NonPressureForce.h"
#include "TimeHelper.h"
#include "SPHSolver.h"
#include "HashTable.h"
#include "../Common/SimulationSystem.h"
#include "SPHStructs.h"

// Loop over neighborhood
#define forall_fluid_neighbors(code)\
    for (auto & pointInfo: grid->table[cellId].neighbors)\
    { \
        uint j = pointInfo.id;\
        uint nfObjId = pointInfo.pointSetId;\
        FluidObject* nfObj = sim->getFluidModel(nfObjId);\
        code\
    }\

#define forall_fluid_neighbors_in_same_phase(code)\
    for (auto & pointInfo: grid->table[cellId].neighbors)\
    {\
        uint j = pointInfo.id;\
        code\
    }\

#define forall_boundary_neighbors(code) \
    for (auto & pointInfo: grid->table[cellId].boundary_neighbors)\
    {\
        uint b = pointInfo.id;\
        uint nbObjId = pointInfo.pointSetId;\
        code\
    }\

#define forsame_boundary_neighbors(code)\
    for (auto & pointInfo: grid->table[cellId].boundary_neighbors)\
    {\
        uint b = pointInfo.id;\
        code\
    }\

/**
 * @brief Clase singleton para controlar la simulacion actual, parametros generales y lectura y escritura en ficheros
 */
class SPHSimulation : public SimulationSystem
{
    private:

        static SPHSimulation* current;

        std::string name;
        SimulationInfo simInfo;

    protected:

        SPHSolver* solver;
        HashTable* grid;
        std::vector<FluidObject*> fluidObjects;
        std::vector<BoundaryObject*> boundaryObjects;

        TimeHelper tm;

        Real particleRadius;
        Real supportRadius;
        Vector3r gravity;

        int current_method;
        int current_bh_method;
        int current_visco_method;
        int current_surften_method;
        int current_adhesion_method;
        int current_kernel_method;
        int current_gradient_method;
        int current_laplacian_method;

        bool activeSave;

    public:

        // Solver methods
        const static int WCSPH_METHOD;
        const static int PCISPH_METHOD;
        const static int DFSPH_METHOD;

        // Boundary methods
        const static int CUBE_BOUNDARY_METHOD;
        const static int PCISPH_BOUNDARY_METHOD;
        const static int AKINCI_BOUNDARY_METHOD;

        // Non pressure forces
        const static int STANDARD_VISCOSITY_METHOD;
        const static int ARTIFICIAL_VISCOSITY_METHOD;
        const static int XSPH_VISCOSITY_METHOD;
        const static int AKINCI_SURFACE_TENSION_METHOD;
        const static int AKINCI_ADHESION_METHOD;

        SPHSimulation();
        ~SPHSimulation();

        static SPHSimulation* getCurrent();
        static void setCurrent(SPHSimulation*);
        static bool hasCurrent();

        void reset() override;
        void init() override;
        void deferredInit() override {}
        bool step() override; // to do: dejar step en void y crear algun método para hacer if (nohaacabado) step()
        void release() override;
        void run();

        void initKernels();
        void initGrid();
        void initMasses();

        // Setters
        void setParticleRadius(Real);
        void setSupportRadius(Real);
        void setGravity(Vector3r gravity) { this->gravity = gravity; }
        void setSimulationMethod(const int);
        void setBoundaryMethod(const int);
        void setViscosityMethod(const int method) { current_visco_method = method; }
        void setSurfaceTensionMethod(const int method) { current_surften_method = method; }
        void setAdhesionMethod(const int method) { current_adhesion_method = method; }
        void activateSave(bool active) { activeSave = active; }
        // to do: gestionar mejor los metodos relacionados con el tiempo
        void setTime(Real time) { tm.setTime(time); }
        void setStartTime(Real startTime) { tm.setStartTime(startTime); }
        void setEndTime(Real endTime) { tm.setEndTime(endTime); }
        void setTimeStep(Real ts) { tm.setTimeStep(ts); }
        void setFPS(Real fps) { tm.setFPS(fps); }
        void setMinTimeStep(Real minTs) { tm.setMinTimeStep(minTs); }
        void setMaxTimeStep(Real maxTs) { tm.setMaxTimeStep(maxTs); }

        // Getters
        Real getParticleRadius() { return particleRadius; }
        Real getSupportRadius() { return supportRadius; }
        Vector3r getGravity() { return gravity; }
        Real getTime() { return tm.getTime(); }
        uint getFrame() { return tm.getFrame(); }
        Real getStartTime() { return tm.getStartTime(); }
        Real getEndTime() { return tm.getEndTime(); }
        Real getTimeStep() { return tm.getTimeStep(); }
        Real getFPS() { return tm.getFPS(); }
        void startCounting(std::string name) { tm.startCounting(name); }
        void stopCounting(std::string name) { tm.stopCounting(name); }
        double getInterval(std::string name) { return tm.getInterval(name); }
        Real getMinTimeStep() { return tm.getMinTimeStep(); }
        Real getMaxTimeStep() { return tm.getMaxTimeStep(); }
        std::string getName() { return name; }
        uint getTotalParticles();
        const BoundaryInfo& getBoundaryInfo() {return simInfo.boundaryData; }

        int getSimulationMethod() { return current_method; }
        int getBoundaryHandlingMethod() { return current_bh_method; }
        int getViscosityMethod() { return current_visco_method; }
        int getSurfaceTensionMethod() { return current_surften_method; }
        int getAdhesionMethod() { return current_adhesion_method; }

        SPHSolver* getSolver() { return solver; }
        HashTable* getGrid() { return grid; }
        const uint numberFluidModels() { return static_cast<uint>(fluidObjects.size()); }
        const uint numberBoundaryModels() { return static_cast<uint>(boundaryObjects.size()); }
        size_t getSize() override 
        { 
            if (boundaryObjects.empty())
            {
                return fluidObjects.size();
            }
            else
            {
                return fluidObjects.size() + boundaryObjects.size() - 1; 
            }
        }
        SimulationObject* getSimObject(uint i) override 
        {
            if (i < fluidObjects.size())
            {
                return fluidObjects[i]; 
            }
            else 
            {
                return boundaryObjects[i - fluidObjects.size() + 1];
            }
        }

        void addFluidModel(FluidObject* fObj);
        void addBoundaryModel(BoundaryObject*);
        FluidObject* getFluidModel (const uint i) { return fluidObjects[i]; } 
        BoundaryObject* getBoundaryModel (const uint i) { return boundaryObjects[i]; }
        
        void computeNonPressureForces();
        void emitParticles();

        void printInfo();

        bool importScene(const std::string&);
        bool importFluidState(uint frame);
        void clearSimInfo();

        // For OpenGL colors
        void updateScalarField(std::vector<std::vector<Real>>& scalarField) override;
};

#endif