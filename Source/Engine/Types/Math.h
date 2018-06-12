#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define MATH_PI 3.141592653f

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;

typedef glm::bvec2 BVector2;
typedef glm::bvec3 BVector3;
typedef glm::bvec4 BVector4;

typedef glm::ivec2 IVector2;
typedef glm::ivec3 IVector3;
typedef glm::ivec4 IVector4;

typedef glm::uvec2 UVector2;
typedef glm::uvec3 UVector3;
typedef glm::uvec4 UVector4;

typedef glm::dvec2 DVector2;
typedef glm::dvec3 DVector3;
typedef glm::dvec4 DVector4;

typedef glm::mat2 Matrix2;
typedef glm::mat3 Matrix3;
typedef glm::mat4 Matrix4;

typedef glm::quat Quaternion;

struct Rectangle
{
	float x, y, width, height;

	Rectangle()
		: x(0.0f)
		, y(0.0f)
		, width(0.0f)
		, height(0.0f)
	{
	}

	Rectangle(float x, float y, float w, float h)
		: x(x)
		, y(y)
		, width(w)
		, height(h)
	{
	}
};