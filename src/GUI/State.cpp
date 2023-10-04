#include "State.h"
#include <iostream>

void State::addMSH(std::string id, MSH* msh)
{
	loadedMSH[id] = msh;
}

bool State::existMSH(std::string id)
{
	auto it = loadedMSH.find(id);
	if (it != loadedMSH.end()) return true;
	return false;
}

MSH* State::getMSH(std::string id)
{
	return loadedMSH[id];
}

void State::showMSH()
{
	std::cout << "Loaded MSHs: " << loadedMSH.size() << std::endl;
	for (auto pair : loadedMSH)
	{
		std::cout << pair.first << ": ";
		for (auto mesh : pair.second->getMeshes())
		{
			std::cout << mesh->getMeshID() << ", ";
		}
		std::cout << std::endl;
	}
}
