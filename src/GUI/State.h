#pragma once

#include "MSH.h"
#include <unordered_map>

class State
{
	inline static std::unordered_map<std::string, MSH*> loadedMSH;

public:
	static void addMSH(std::string id, MSH* msh);
	static bool existMSH(std::string id);
	static MSH* getMSH(std::string id);
	static void showMSH();
};

