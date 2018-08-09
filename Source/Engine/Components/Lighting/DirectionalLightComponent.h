#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/MaterialRenderData.h"
#include "Engine/Types/OrientedBounds.h"

#include "Engine/ECS/Component.h"

// Describes a directional light.
struct DirectionalLightComponent
{
	bool isShadowCasting = false;
	int shadowMapSize = 512;
	int shadowMapCascades = 1;

	std::shared_ptr<IGraphicsImage> shadowMapImage;
	std::shared_ptr<IGraphicsImageView> shadowMapImageView;
	std::shared_ptr<IGraphicsFramebuffer> shadowMapFramebuffer;
	Frustum frustum;
};
