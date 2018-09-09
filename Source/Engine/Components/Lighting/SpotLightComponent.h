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

// Stores information on a spot lights shadow.
struct SpotLightShadowInfo
{
	DrawViewState drawViewState;

	Frustum frustum;
	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;

	std::shared_ptr<IGraphicsImage> shadowMapImage;

	std::shared_ptr<IGraphicsImageView> shadowMapImageView;
	std::shared_ptr<IGraphicsFramebuffer> shadowMapFramebuffer;
	std::shared_ptr<IGraphicsSampler> shadowMapSampler;

	RenderPropertyCollection viewProperties;
	RenderPropertyCollection shadowMaskProperties;
};

// Describes a spotlight
struct SpotLightComponent
{
	// If true this light will cast shadows into the scene.
	bool isShadowCasting = false;

	// Size of shadow map texture allocated for each cascade.
	int shadowMapSize = 512;

	// Angle of the outer cone of the light projection.
	float angle = 45.0f;

	// Distance in world units that the spot lights influence will decay over.
	float range = 1000.0f;

	SpotLightShadowInfo shadowInfo;
};
