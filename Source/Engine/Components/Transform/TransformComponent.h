#pragma once
#include "Pch.h"

#include "Engine/Types/Math.h"
#include "Engine/Components/Component.h"

// Holds data required to position an entity in 3d space.
struct TransformComponent
{
	ComponentRef<TransformComponent> parent;

	Quaternion localRotation;
	Vector3 localPosition;
	Vector3 localScale;

	Matrix4 localToWorld;
	Matrix4 worldToLocal;

	bool isDirty;
	uint64_t version;

	Array<ComponentRef<TransformComponent>> children;
};

// Sets a transform components position.
struct SetTransformMessage
{
	ComponentRef<TransformComponent> component;

	Quaternion localRotation;
	Vector3 localPosition;
	Vector3 localScale;
};

// Sets a transform components parent.
struct SetTransformParentMessage
{
	ComponentRef<TransformComponent> component;

	ComponentRef<TransformComponent> parent;
};