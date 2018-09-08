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

	int mapSize;

	float splitMinDistance;
	float splitMaxDistance;

	float worldRadius;

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
	// If true this light will cast shadows into the scene.
	bool isShadowCasting = false;

	// Size of shadow map texture allocated for each cascade.
	int shadowMapSize = 512;

	// Number of shadow cascades the view frustum is split into for shadow rendering.
	int shadowMapCascades = 4;

	// 0 to 1, the lower the value the closer to a linear split the cascades will have.
	float shadowMapSplitExponent = 0.9f; 

	// Maximum distance shadows are rendered to, and thus what cascades are fitted to.
	float shadowDistance = 512;

	// Fraction of shadow cascade which will fade in/out to blend between cascades.
	float shadowMapCascadeBlendFactor = 0.05f; 

	RenderPropertyCollection shadowMaskProperties;
	Array<DirectionalLightCascadeInfo> shadowMapCascadeInfo;
};
