#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Rendering/MeshRenderState.h"
#include "Engine/Types/OrientedBounds.h"
#include "Engine/Types/Frustum.h"

#include "Engine/Rendering/MeshBatcher.h"
#include "Engine/Rendering/Renderer.h"

#include "Engine/Types/OctTree.h"

#include "Engine/ECS/Component.h"

class IGraphicsCommandBuffer;

// Stores per-face information on a point lights shadow map.
struct PointLightShadowFaceInfo
{
	DrawViewState drawViewState;

	Frustum frustum;
	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;

	std::shared_ptr<IGraphicsImageView> shadowMapImageView;
	std::shared_ptr<IGraphicsFramebuffer> shadowMapFramebuffer;

	RenderPropertyCollection viewProperties;
};

// Stores information on a point lights shadow
struct PointLightShadowInfo
{
	PointLightShadowFaceInfo faceInfo[6];

	std::shared_ptr<IGraphicsImage> shadowMapImage;
	std::shared_ptr<IGraphicsSampler> shadowMapSampler;
	std::shared_ptr<IGraphicsImageView> shadowMapSamplerImageView;
	
	RenderPropertyCollection shadowMaskProperties;
};

// Describes a point light
struct PointLightComponent
{
	// If true this light will cast shadows into the scene.
	bool isShadowCasting = false;

	// Size of shadow map texture allocated for each cascade.
	int shadowMapSize = 512;

	// Radius of point lights influence.
	float radius = 1000.0f;

	PointLightShadowInfo shadowInfo;
};
