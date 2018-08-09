#pragma once
#include "Pch.h"

#include "Engine/Types/Rectangle.h"

// Describes the fly camera movement behaviour.
struct FlyCameraComponent
{
	float movementSpeed = 100.0f;
	float mouseSensitivity = 100.0f;

	Vector3 viewRotationEuler = Vector3::Zero;
};

// Changes the movement configuration of a fly camera component.
struct SetFlyCameraConfigMessage
{
	ComponentRef<FlyCameraComponent> component;

	float movementSpeed = 100.0f;
	float mouseSensitivity = 100.0f;
};
