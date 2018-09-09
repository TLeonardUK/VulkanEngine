#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderSpotLightShadowMapSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Mesh/MeshRenderStateUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Types/OctTree.h"

RenderSpotLightShadowMapSystem::RenderSpotLightShadowMapSystem(
	std::shared_ptr<World> world,
	std::shared_ptr<Logger> logger,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<ResourceManager> resourceManager,
	std::shared_ptr<IGraphics> graphics
)
	: m_renderer(renderer)
	, m_graphics(graphics)
	, m_logger(logger)
{
	AddPredecessor<MeshRenderStateUpdateSystem>();
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();
}

void RenderSpotLightShadowMapSystem::RenderLight(
	World& world,
	const FrameTime& frameTime,
	SpotLightComponent* light,
	const TransformComponent* transform)
{
	Vector3 lightWorldLocation = TransformUtils::GetWorldLocation(transform);
	Vector3 lightWorldDirection = TransformUtils::GetForwardVector(transform);
	Matrix4 lightViewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + lightWorldDirection, Vector3::Up);

	// Create resources if required.
	if (light->shadowInfo.shadowMapImage == nullptr)
	{
		light->shadowInfo.shadowMapImage = m_graphics->CreateImage("Spot Light Shadow Map", light->shadowMapSize, light->shadowMapSize, 1, GraphicsFormat::SFLOAT_D32, false, GraphicsUsage::DepthAttachment);
		light->shadowInfo.shadowMapImageView = m_graphics->CreateImageView("Spot Light Shadow Map View", light->shadowInfo.shadowMapImage);

		// Create framebuffer.
		GraphicsFramebufferSettings settings;
		settings.width = light->shadowMapSize;
		settings.height = light->shadowMapSize;
		settings.renderPass = m_renderer->GetRenderPassForTarget(FrameBufferTarget::DepthBuffer);
		settings.attachments.push_back(light->shadowInfo.shadowMapImageView);

		light->shadowInfo.shadowMapFramebuffer = m_graphics->CreateFramebuffer("Spot Light Shadow Map Framebuffer", settings);

		SamplerDescription samplerDescription;
#if 0
		samplerDescription.MagnificationFilter = GraphicsFilter::NearestNeighbour;
		samplerDescription.MinificationFilter = GraphicsFilter::NearestNeighbour;
		samplerDescription.AddressModeU = GraphicsAddressMode::ClampToEdge;
		samplerDescription.AddressModeV = GraphicsAddressMode::ClampToEdge;
		samplerDescription.AddressModeW = GraphicsAddressMode::ClampToEdge;
		samplerDescription.BorderColor = GraphicsBorderColor::OpaqueWhite_Float;
#else
		samplerDescription.MagnificationFilter = GraphicsFilter::Linear;
		samplerDescription.MinificationFilter = GraphicsFilter::Linear;
		samplerDescription.AddressModeU = GraphicsAddressMode::ClampToBorder;
		samplerDescription.AddressModeV = GraphicsAddressMode::ClampToBorder;
		samplerDescription.AddressModeW = GraphicsAddressMode::ClampToBorder;
		samplerDescription.BorderColor = GraphicsBorderColor::OpaqueWhite_Float;
#endif
		light->shadowInfo.shadowMapSampler = m_graphics->CreateSampler("Spot Light Shadow Map Sampler", samplerDescription);
	}

	// Our projection matrix is orthographic, aligned to the light direction, and encapsulates the entire spotlight cone.
	if (!m_renderer->IsRenderingFrozen())
	{
		light->shadowInfo.viewMatrix = lightViewMatrix;
		light->shadowInfo.projectionMatrix = Matrix4::Perspective(
			light->angle,
			1.0f,
			1.0f,
			light->range);
	}

	Matrix4 shadowMatrix = light->shadowInfo.projectionMatrix * light->shadowInfo.viewMatrix;
	light->shadowInfo.frustum = Frustum(shadowMatrix);

	if (false)//m_renderer->IsDrawBoundsEnabled())
	{
		DrawDebugFrustumMessage frusumDrawMsg;
		frusumDrawMsg.frustum = light->shadowInfo.frustum;
		frusumDrawMsg.color = Color::Amber;
		world.QueueMessage(frusumDrawMsg);
	}

	m_renderer->DisplayDebugFrameBuffer(light->shadowInfo.shadowMapImageView);

	// Setup rendering properties.
	light->shadowInfo.viewProperties.Set(ViewMatrixHash, light->shadowInfo.viewMatrix);
	light->shadowInfo.viewProperties.Set(ProjectionMatrixHash, light->shadowInfo.projectionMatrix);
	light->shadowInfo.viewProperties.UpdateResources(m_graphics, m_logger);

	// Draw view to frame buffer.
	light->shadowInfo.drawViewState.world = &world;
	light->shadowInfo.drawViewState.frustum = light->shadowInfo.frustum;
	light->shadowInfo.drawViewState.viewProperties = &light->shadowInfo.viewProperties;
	light->shadowInfo.drawViewState.name = "Spot Light Shadow Map";
	light->shadowInfo.drawViewState.stage = RenderCommandStage::Global_ShadowMap;
	light->shadowInfo.drawViewState.viewport = Rect(0, 0, light->shadowMapSize, light->shadowMapSize);
	light->shadowInfo.drawViewState.viewId = 0;
	light->shadowInfo.drawViewState.requiredFlags = RenderFlags::ShadowCaster;
	light->shadowInfo.drawViewState.materialVariant = MaterialVariant::DepthOnly;
	light->shadowInfo.drawViewState.framebuffer = light->shadowInfo.shadowMapFramebuffer;
	m_renderer->DrawView(light->shadowInfo.drawViewState);

	// Transition and clear the shadow map.
	{
		std::shared_ptr<IGraphicsCommandBuffer> clearBuffer = m_renderer->RequestPrimaryBuffer();
		clearBuffer->Reset();
		clearBuffer->Begin();

		clearBuffer->TransitionResource(light->shadowInfo.shadowMapImage, GraphicsAccessMask::ReadWrite);
		clearBuffer->Clear(light->shadowInfo.shadowMapImage, Color::Black, 1.0f, 0.0f);

		clearBuffer->End();

		m_renderer->QueuePrimaryBuffer("Shadow Map Clear", RenderCommandStage::Global_PreRender, clearBuffer, 0);
	}

	// Transition back to reading format.
	{
		std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();
		drawBuffer->Reset();
		drawBuffer->Begin();

		drawBuffer->TransitionResource(light->shadowInfo.shadowMapImage, GraphicsAccessMask::Read);

		drawBuffer->End();

		m_renderer->QueuePrimaryBuffer("Shadow Map Transition", RenderCommandStage::Global_PreViews, drawBuffer, 0);
	}

	// Update shadow mask rendering properties.
	light->shadowInfo.shadowMaskProperties.Set(LightShadowMapHash, RenderPropertyImageSamplerValue(light->shadowInfo.shadowMapImageView, light->shadowInfo.shadowMapSampler));
	light->shadowInfo.shadowMaskProperties.Set(LightWorldDirectionHash, Vector4(lightWorldDirection, 1.0f));
	light->shadowInfo.shadowMaskProperties.Set(LightShadowMapSizeHash, (float)light->shadowMapSize);
	light->shadowInfo.shadowMaskProperties.Set(LightShadowViewProjHash, shadowMatrix);

	light->shadowInfo.shadowMaskProperties.UpdateResources(m_graphics, m_logger);
}

void RenderSpotLightShadowMapSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<SpotLightComponent*>& lights,
	const Array<const TransformComponent*>& transforms)
{
	// Draw shadow map for each light.
	ProfileScope scope(Color::Blue, "Update Spot Light Shadow Maps");

	ParallelFor(entities.size(), [&](int i) {

		SpotLightComponent* light = lights[i];
		const TransformComponent* transform = transforms[i];

		if (light->isShadowCasting)
		{
			RenderLight(world, frameTime, light, transform);
		}

	}, 1, "Spot Light Shadow Map Building");
}
