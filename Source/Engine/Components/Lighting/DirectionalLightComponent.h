#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Rendering/MeshRenderState.h"
#include "Engine/Types/OrientedBounds.h"
#include "Engine/Types/Frustum.h"

#include "Engine/Rendering/MeshBatcher.h"

#include "Engine/Types/OctTree.h"

#include "Engine/ECS/Component.h"

class IGraphicsCommandBuffer;

// Stores information on an individual cascade in a shadow-map.
struct DirectionalLightCascadeInfo
{
	std::shared_ptr<IGraphicsImage> shadowMapImage;
	std::shared_ptr<IGraphicsImageView> shadowMapImageView;
	std::shared_ptr<IGraphicsFramebuffer> shadowMapFramebuffer;
	std::shared_ptr<IGraphicsSampler> shadowMapSampler;

	float splitMinDistance;
	float splitMaxDistance;

	Matrix4 projectionMatrix;
	Frustum viewFrustum;
	Frustum frustum;

	MeshBatcher meshBatcher;
	OctTree<Entity>::Result visibleEntitiesResult;
	Array<std::shared_ptr<IGraphicsCommandBuffer>> batchBuffers;

	RenderPropertyCollection viewProperties;
};

// Describes a directional light.
struct DirectionalLightComponent
{
	bool isShadowCasting = false;
	int shadowMapSize = 512;
	int shadowMapCascades = 4;
	float shadowMapSplitExponent = 0.8f; // 0 to 1, the lower the value the closer to a linear split the cascades will have.
	float shadowDistance = 512;

	RenderPropertyCollection shadowMaskProperties;
	Array<DirectionalLightCascadeInfo> shadowMapCascadeInfo;
};
