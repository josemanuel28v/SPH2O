#ifndef _SCENE_LOADER_H_
#define _SCENE_LOADER_H_

#include <string>
#include <vector>

#include "json/json.hpp"
#include "SPHStructs.h"

class SceneLoader
{
    public:

        static bool readConfiguration(SimulationInfo&, const std::string& path);
        static void readSceneInfo(SimulationInfo&, const nlohmann::json& file);
        static void readFluidInfo(SimulationInfo&, const nlohmann::json& file);
        static void readBoundaryInfo(SimulationInfo&, const nlohmann::json& file);
        static void writeFluid();

        const inline static uint MAX_DIGITS = 4;
};

#endif
