#ifndef _NUMERIC_TYPES_H_
#define _NUMERIC_TYPES_H_

#ifdef _WIN32
#include <corecrt_math_defines.h>
typedef unsigned int uint;
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/string_cast.hpp>

typedef float Real;
typedef glm::vec3 Vector3r;
typedef glm::vec4 Vector4r;
typedef glm::ivec3 Vector3i;
typedef glm::uvec3 Vector3u;
typedef glm::mat4 Matrix4r;
typedef glm::quat Quat4r;
typedef glm::ivec3 Vector3i;

#endif