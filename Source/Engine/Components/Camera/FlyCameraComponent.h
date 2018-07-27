#pragma once
#include "Pch.h"

#include "Engine/Types/Rectangle.h"

// Describes the fly camera movement behaviour.
struct FlyCameraComponent
{
	float movementSpeed;
	float mouseSensitivity;

	Vector3 viewRotationEuler;
};

// Changes the movement configuration of a fly camera component.
struct SetFlyCameraConfigMessage
{
	ComponentRef<FlyCameraComponent> component;

	float movementSpeed;
	float mouseSensitivity;
};
