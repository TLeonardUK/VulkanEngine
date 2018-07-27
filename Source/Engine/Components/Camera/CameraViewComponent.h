#pragma once
#include "Pch.h"

#include "Engine/Types/Rectangle.h"
#include "Engine/Types/Frustum.h"

// Describes a camera viewport that will be rendered during the frame.
struct CameraViewComponent
{
	Rect viewport;

	float depthMin;
	float depthMax;

	float fov;

	Frustum frustum;
};

// Changes the projection settings of a camera view.
struct SetCameraViewProjectionMessage
{
	ComponentRef<CameraViewComponent> component;

	Rect viewport;

	float depthMin;
	float depthMax;

	float fov;
};
