#pragma once
#include "Pch.h"

#include "Engine/Types/Math.h"
#include "Engine/Components/Transform/TransformComponent.h"

namespace TransformUtils {

Vector3 GetWorldLocation(const TransformComponent* component);
Quaternion GetWorldRotation(const TransformComponent* component);
Vector3 GetWorldScale(const TransformComponent* component);

Vector3 GetForwardVector(const TransformComponent* component);
Vector3 GetRightVector(const TransformComponent* component);
Vector3 GetUpVector(const TransformComponent* component);

Vector3 TransformLocation(const TransformComponent* component, const Vector3& location);
Vector3 TransformVector(const TransformComponent* component, const Vector3& vector);
Vector3 TransformDirection(const TransformComponent* component, const Vector3& direction); // Same as TransformsVector but unaffected by scale.

Vector3 InverseTransformLocation(const TransformComponent* component, const Vector3& location);
Vector3 InverseTransformVector(const TransformComponent* component, const Vector3& vector);
Vector3 InverseTransformDirection(const TransformComponent* component, const Vector3& direction); // Same as TransformsVector but unaffected by scale.

}; // namespace TransformUtils