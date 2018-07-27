#include "Pch.h"

#include "Engine/Components/Transform/TransformUtils.h"

#include "glm/glm.hpp"

namespace TransformUtils {

Vector3 GetWorldLocation(const TransformComponent* component)
{
	return Vector3::Zero * component->localToWorld;
}

Quaternion GetWorldRotation(const TransformComponent* component)
{
	return component->localToWorld.ToQuaternion();
}

Vector3 GetWorldScale(const TransformComponent* component)
{
	return component->localToWorld.ExtractScale();
}

Vector3 GetForwardVector(const TransformComponent* component)
{
	return InverseTransformVector(component, Vector3::Forward);
}

Vector3 GetRightVector(const TransformComponent* component)
{
	return InverseTransformVector(component, Vector3::Right);
}

Vector3 GetUpVector(const TransformComponent* component)
{
	return InverseTransformVector(component, Vector3::Up);
}

Vector3 TransformDirection(const TransformComponent* component, const Vector3& direction)
{
	return component->localToWorld.TransformDirection(direction);
}

Vector3 TransformLocation(const TransformComponent* component, const Vector3& location)
{
	return component->localToWorld.TransformLocation(location);
}

Vector3 TransformVector(const TransformComponent* component, const Vector3& vector)
{
	return component->localToWorld.TransformVector(vector);
}

Vector3 InverseTransformDirection(const TransformComponent* component, const Vector3& direction)
{
	return component->worldToLocal.TransformDirection(direction);
}

Vector3 InverseTransformLocation(const TransformComponent* component, const Vector3& location)
{
	return component->worldToLocal.TransformLocation(location);
}

Vector3 InverseTransformVector(const TransformComponent* component, const Vector3& vector)
{
	return component->worldToLocal.TransformVector(vector);
}

}; // namespace TransformUtils