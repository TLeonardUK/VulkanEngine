#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderPointLightShadowMapSystem.h"
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

RenderPointLightShadowMapSystem::RenderPointLightShadowMapSystem(
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

void RenderPointLightShadowMapSystem::RenderLight(
	World& world,
	const FrameTime& frameTime,
	PointLightComponent* light,
	const TransformComponent* transform)
{
	Vector3 lightWorldLocation = TransformUtils::GetWorldLocation(transform);
	Vector3 lightWorldForward = TransformUtils::GetForwardVector(transform);
	Vector3 lightWorldRight = TransformUtils::GetRightVector(transform);
	Vector3 lightWorldUp = TransformUtils::GetUpVector(transform);

	// Create resources if required.
	if (light->shadowInfo.shadowMapImage == nullptr)
	{
		light->shadowInfo.shadowMapImage = m_graphics->CreateImage("Point Light Shadow Map", light->shadowMapSize, light->shadowMapSize, 6, GraphicsFormat::SFLOAT_D32, false, GraphicsUsage::DepthAttachment);

		for (int i = 0; i < 6; i++)
		{
			PointLightShadowFaceInfo& face = light->shadowInfo.faceInfo[i];

			face.shadowMapImageView = m_graphics->CreateImageView(StringFormat("Point Light Shadow Map View (Face %i)", i), light->shadowInfo.shadowMapImage, i, 1);

			// Create framebuffer.
			GraphicsFramebufferSettings settings;
			settings.width = light->shadowMapSize;
			settings.height = light->shadowMapSize;
			settings.renderPass = m_renderer->GetRenderPassForTarget(FrameBufferTarget::DepthBuffer);
			settings.attachments.push_back(face.shadowMapImageView);

			face.shadowMapFramebuffer = m_graphics->CreateFramebuffer(StringFormat("Point Light Shadow Map Framebuffer (Face %i)", i), settings);
		}

		// Create sampler.
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
		light->shadowInfo.shadowMapSampler = m_graphics->CreateSampler("Point Light Shadow Map Sampler", samplerDescription);

		light->shadowInfo.shadowMapSamplerImageView = m_graphics->CreateImageView("Point Light Shadow Map Sampler View", light->shadowInfo.shadowMapImage);
	}

	// Render each face of the point lights shadow map.

	// Our projection matrix is orthographic, aligned to the light direction, and encapsulates the entire spotlight cone.
	if (!m_renderer->IsRenderingFrozen())
	{
		light->shadowInfo.faceInfo[0].viewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + Vector3( 1.0f,  0.0f,  0.0f), Vector3( 0.0f, 1.0f,  0.0f));
		light->shadowInfo.faceInfo[1].viewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + Vector3(-1.0f,  0.0f,  0.0f), Vector3( 0.0f, 1.0f,  0.0f));
		light->shadowInfo.faceInfo[2].viewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + Vector3( 0.0f,  1.0f,  0.0f), Vector3( 0.0f,  0.0f, -1.0f));
		light->shadowInfo.faceInfo[3].viewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + Vector3( 0.0f, -1.0f,  0.0f), Vector3( 0.0f,  0.0f, 1.0f));
		light->shadowInfo.faceInfo[4].viewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + Vector3( 0.0f,  0.0f,  1.0f), Vector3( 0.0f, 1.0f,  0.0f));
		light->shadowInfo.faceInfo[5].viewMatrix = Matrix4::LookAt(lightWorldLocation, lightWorldLocation + Vector3( 0.0f,  0.0f, -1.0f), Vector3( 0.0f, 1.0f,  0.0f));

		for (int i = 0; i < 6; i++)
		{
			PointLightShadowFaceInfo& face = light->shadowInfo.faceInfo[i];

			face.projectionMatrix = Matrix4::Perspective(
				Math::Pi*0.5f,
				1.0f,
				1.0f,
				light->radius);

			face.frustum = Frustum(face.projectionMatrix * face.viewMatrix);
		}
	}

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

	ParallelFor(6, [&](int i) {

		PointLightShadowFaceInfo& face = light->shadowInfo.faceInfo[i];

		if (m_renderer->IsDrawBoundsEnabled())
		{
			DrawDebugFrustumMessage frusumDrawMsg;
			frusumDrawMsg.frustum = face.frustum;
			frusumDrawMsg.color = Color::Amber;
			world.QueueMessage(frusumDrawMsg);
		}

		m_renderer->DisplayDebugFrameBuffer(face.shadowMapImageView);

		// Setup rendering properties.
		face.viewProperties.Set(ViewMatrixHash, face.viewMatrix);
		face.viewProperties.Set(ProjectionMatrixHash, face.projectionMatrix);
		face.viewProperties.Set(DistanceOriginHash, Vector4(lightWorldLocation, 1.0f));
		face.viewProperties.Set(MaxDistanceHash, light->radius);
		face.viewProperties.UpdateResources(m_graphics, m_logger);

		// Draw view to frame buffer.
		face.drawViewState.world = &world;
		face.drawViewState.frustum = face.frustum;
		face.drawViewState.viewProperties = &face.viewProperties;
		face.drawViewState.name = "Spot Light Shadow Map";
		face.drawViewState.stage = RenderCommandStage::Global_ShadowMap;
		face.drawViewState.viewport = Rect(0, 0, light->shadowMapSize, light->shadowMapSize);
		face.drawViewState.viewId = 0;
		face.drawViewState.requiredFlags = RenderFlags::ShadowCaster;
		face.drawViewState.materialVariant = MaterialVariant::NormalizedDistance;
		face.drawViewState.framebuffer = face.shadowMapFramebuffer;
		m_renderer->DrawView(face.drawViewState);

	}, 1, "Point Light Shadow Map Face Rendering");

	// Update shadow mask rendering properties.
	light->shadowInfo.shadowMaskProperties.Set(LightShadowMapHash, RenderPropertyImageSamplerValue(light->shadowInfo.shadowMapSamplerImageView, light->shadowInfo.shadowMapSampler));
	light->shadowInfo.shadowMaskProperties.Set(LightWorldLocationHash, Vector4(lightWorldLocation, 1.0f));
	light->shadowInfo.shadowMaskProperties.Set(LightMaxDistanceHash, light->radius);
	light->shadowInfo.shadowMaskProperties.Set(LightShadowMapSizeHash, (float)light->shadowMapSize);

	light->shadowInfo.shadowMaskProperties.UpdateResources(m_graphics, m_logger);
}

void RenderPointLightShadowMapSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<PointLightComponent*>& lights,
	const Array<const TransformComponent*>& transforms)
{
	// Draw shadow map for each light.
	ProfileScope scope(Color::Blue, "Update Point Light Shadow Maps");

	ParallelFor(entities.size(), [&](int i) {

		PointLightComponent* light = lights[i];
		const TransformComponent* transform = transforms[i];

		if (light->isShadowCasting)
		{
			RenderLight(world, frameTime, light, transform);
		}

	}, 1, "Point Light Shadow Map Building");
}
