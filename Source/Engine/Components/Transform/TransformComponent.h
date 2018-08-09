#pragma once
#include "Pch.h"

#include "Engine/Types/Math.h"
#include "Engine/ECS/Component.h"

// Holds data required to position an entity in 3d space.
struct TransformComponent
{
	ComponentRef<TransformComponent> parent = NoEntity;

	Quaternion localRotation = Quaternion::Identity;
	Vector3 localPosition = Vector3::Zero;
	Vector3 localScale = Vector3::One;

	Matrix4 localToWorld = Matrix4::Identity;
	Matrix4 worldToLocal = Matrix4::Identity;

	bool isDirty = true;
	uint64_t version = 0;

	Array<ComponentRef<TransformComponent>> children;
};

// Sets a transform components position.
struct SetTransformMessage
{
	ComponentRef<TransformComponent> component = NoEntity;

	Quaternion localRotation = Quaternion::Identity;
	Vector3 localPosition = Vector3::Zero;
	Vector3 localScale = Vector3::One;
};

// Sets a transform components parent.
struct SetTransformParentMessage
{
	ComponentRef<TransformComponent> component = NoEntity;

	ComponentRef<TransformComponent> parent = NoEntity;
};