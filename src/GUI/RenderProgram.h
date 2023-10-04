#pragma once
#define GLAD_ONLY_HEADERS
#include "common.h"
#include "../Common/numeric_types.h"
#include <unordered_map>

class RenderProgram
{
public:
	enum class renderTypes_e {
		vertex = 0, fragment = 1
	};

	std::unordered_map<std::string, uint> varList;

	virtual ~RenderProgram() {}
	virtual void setProgram(std::string programSrc, renderTypes_e type) = 0;
	virtual void linkPrograms() = 0;
	virtual std::string getErrorMsg(GLuint shaderID) = 0;
	virtual void use() = 0;
	virtual void setInt(std::string, int val) = 0;
	virtual void setFloat(std::string loc, float val) = 0;
	virtual void setVec3(std::string loc, const glm::vec3& vec) = 0;
	virtual void setVec4(std::string loc, const glm::vec4& vec) = 0;
	virtual void setMatrix(std::string loc, const glm::mat4& matrix) = 0;
	virtual void setTexture2D(std::string loc, uint textUnit) = 0;

protected:
	uint programId = 0;
};

