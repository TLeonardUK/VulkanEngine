#pragma once
#include "Pch.h"

#include "Engine/Types/Rectangle.h"
#include "Engine/Types/Frustum.h"

// Describes a camera viewport that will be rendered during the frame.
struct CameraViewComponent
{
	Rect viewport = Rect::Empty;

	float depthMin = 1.0f;
	float depthMax = 1000.0f;

	float fov = 45.0f;

	Frustum frustum = Frustum::Empty;
	Matrix4 viewMatrix = Matrix4::Identity;
	Matrix4 projectionMatrix = Matrix4::Identity;

	RenderPropertyCollection viewProperties;
};

// Changes the projection settings of a camera view.
struct SetCameraViewProjectionMessage
{
	ComponentRef<CameraViewComponent> component = NoEntity;

	Rect viewport = Rect::Empty;

	float depthMin = 1.0f;
	float depthMax = 1000.0f;

	float fov = 45.0f;
};
