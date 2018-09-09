#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderDirectionalShadowMapSystem.h"
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

float CalculateCascadeDistance(float nearZ, float farZ, float percent, float blend)
{
	float uni = Math::Lerp(nearZ, farZ, percent);  
	float log = nearZ * Math::Pow((farZ / nearZ), percent);
	return Math::Lerp(uni, log, blend);
}

RenderDirectionalShadowMapSystem::RenderDirectionalShadowMapSystem(
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

	// DEBUG DEBUG DEBUG
	// Remove when we figure out why shadow-map it flickers.
	//AddPredecessor<RenderCameraViewSystem>();
	// DEBUG DEBUG DEBUG

	m_meshComponentAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
	m_cameraViewAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(CameraViewComponent) });
}

void RenderDirectionalShadowMapSystem::RenderLight(
	World& world,
	const FrameTime& frameTime,
	DirectionalLightComponent* light,
	CameraViewComponent* view,
	const TransformComponent* transform,
	const TransformComponent* viewTransform)
{
	SpatialIndexSystem* spatialSystem = world.GetSystem<SpatialIndexSystem>();

	Vector3 lightWorldDirection = TransformUtils::GetForwardVector(transform);

	Matrix4 viewMatrix = Matrix4::LookAt(Vector3::Zero, TransformUtils::GetForwardVector(transform), Vector3::Up);

	Vector3 viewWorldLocation = TransformUtils::GetWorldLocation(viewTransform);
	Vector3 viewForward = TransformUtils::GetForwardVector(viewTransform);

	// Calculate cascades based on view frustum.
	float cascadeNearZ = view->depthMin;
	float cascadeFarZ = Math::Min(view->depthMax, light->shadowDistance);

	light->shadowMapCascadeInfo.resize(light->shadowMapCascades);
	for (int i = 0; i < light->shadowMapCascades; i++)
	{
		DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[i];

		// Create resources if required.
		if (cascade.shadowMapImage == nullptr)
		{
			cascade.mapSize = light->shadowMapSize;

			// Double size of last cascade as it encompases a far larger area.
			if (i == light->shadowMapCascades - 1)
			{
				cascade.mapSize *= 2;
			}

			cascade.shadowMapImage = m_graphics->CreateImage(StringFormat("Directional Shadow Map (Cascade %i)", i), cascade.mapSize, cascade.mapSize, 1, GraphicsFormat::SFLOAT_D32, false, GraphicsUsage::DepthAttachment);
			cascade.shadowMapImageView = m_graphics->CreateImageView(StringFormat("Directional Shadow Map View (Cascade %i)", i), cascade.shadowMapImage);

			// Create framebuffer.
			GraphicsFramebufferSettings settings;
			settings.width = cascade.mapSize;
			settings.height = cascade.mapSize;
			settings.renderPass = m_renderer->GetRenderPassForTarget(FrameBufferTarget::DepthBuffer);
			settings.attachments.push_back(cascade.shadowMapImageView);

			cascade.shadowMapFramebuffer = m_graphics->CreateFramebuffer(StringFormat("Directional Shadow Map Framebuffer (Cascade %i)", i), settings);

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
			cascade.shadowMapSampler = m_graphics->CreateSampler(StringFormat("Directional Shadow Map Sampler (Cascade %i)", i), samplerDescription);
		}

		float minDelta = (1.0f / light->shadowMapCascades) * (i);
		float maxDelta = (1.0f / light->shadowMapCascades) * (i + 1);
		double minDistance = CalculateCascadeDistance(cascadeNearZ, cascadeFarZ, minDelta, light->shadowMapSplitExponent);
		double maxDistance = CalculateCascadeDistance(cascadeNearZ, cascadeFarZ, maxDelta, light->shadowMapSplitExponent);

		cascade.viewFrustum = view->frustum.GetCascade(minDistance, maxDistance);
		Frustum viewSpaceFrustum = view->viewSpaceFrustum.GetCascade(minDistance, maxDistance);

		// Calculate bounds of view space frustum.
		Vector3 corners[(int)FrustumCorners::Count];
		viewSpaceFrustum.GetCorners(corners);
		cascade.splitMinDistance = minDistance;
		cascade.splitMaxDistance = maxDistance;

		// Get world space bounds of frustum.
		Bounds bounds(corners, (int)FrustumCorners::Count);

		// Calculate sphere that fits inside orthographic bounds.
#if 1
		float radius = Math::Min(bounds.GetExtents().x, bounds.GetExtents().y);
		cascade.worldRadius = radius;

		Sphere sphere;
		sphere.origin = cascade.viewFrustum.GetCenter() * viewMatrix;
		sphere.radius = radius;
#else
		Sphere sphere(corners, (int)FrustumCorners::Count);
		sphere.origin = cascade.viewFrustum.GetCenter() * viewMatrix;

#endif

		// Calculate sphere that encompases all corners (we do this as a sphere 
		// is rotationally invariant so we don't get artifacts as the view area changes).
		// update: don't do it this way, we want a sphere that is contained within frustum bounds, not one that
		// containst the frustum (as we will end up rendering a ton of stuff thats not visible to the user).
		//Sphere worldSphere(corners, (int)FrustumCorners::Count);
//		sphere.origin = cascade.viewFrustum.GetCenter() * viewMatrix;
//		sphere.radius *= 0.5f;


		// Calculate bounding box in light-space.
		Bounds lightSpaceBounds = sphere.GetBounds();
		//Bounds worldBounds = worldSphere.GetBounds();

		// Calculate projection matrix that encompases the light space bounds.
		Vector3 originLightSpace = view->frustum.GetOrigin() * viewMatrix;

		float minZ = -originLightSpace.z - 10000.0f; // todo: Shouldn't attach this to frustum. Calculate scene bounds?
		float maxZ = -originLightSpace.z + 10000.0f;

		cascade.projectionMatrix = Matrix4::Orthographic(
			lightSpaceBounds.min.x,
			lightSpaceBounds.max.x,
			lightSpaceBounds.min.y,
			lightSpaceBounds.max.y,
			minZ, 
			maxZ);

		// Create the rounding matrix, by projecting the world-space origin and determining
		// the fractional offset in texel space
		Matrix4 shadowMatrix = cascade.projectionMatrix * viewMatrix;
		Vector4 shadowOrigin = shadowMatrix.TransformVector(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		shadowOrigin = shadowOrigin * cascade.mapSize / 2.0f;

		Vector4 roundedOrigin = shadowOrigin.Round();
		Vector4 roundOffset = roundedOrigin - shadowOrigin;
		roundOffset = roundOffset * 2.0f / cascade.mapSize;
		roundOffset.z = 0.0f;
		roundOffset.w = 0.0f;

		Matrix4 shadowProj = cascade.projectionMatrix;
		shadowProj.SetColumn(3, shadowProj.GetColumn(3) + roundOffset);
		cascade.projectionMatrix = shadowProj;

		// Calculate frustum from projection matrix.
		cascade.frustum = Frustum(cascade.projectionMatrix * viewMatrix);

		if (false)//m_renderer->IsDrawBoundsEnabled())
		{
			{
				DrawDebugFrustumMessage frusumDrawMsg;
				frusumDrawMsg.frustum = cascade.frustum;
				frusumDrawMsg.color = Color::Amber;
				world.QueueMessage(frusumDrawMsg);
			}
		}

		m_renderer->DisplayDebugFrameBuffer(cascade.shadowMapImageView);
	}

	// Batch and render each cascade in parallel.
	ParallelFor(light->shadowMapCascades, [&](int index) {

		DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[index];

		cascade.viewProperties.Set(ViewMatrixHash, viewMatrix);
		cascade.viewProperties.Set(ProjectionMatrixHash, cascade.projectionMatrix);
		cascade.viewProperties.UpdateResources(m_graphics, m_logger);

		// Draw view to frame buffer.
		cascade.drawViewState.world = &world;
		cascade.drawViewState.frustum = cascade.frustum;
		cascade.drawViewState.viewProperties = &cascade.viewProperties;
		cascade.drawViewState.name = "Shadow Map Cascade";
		cascade.drawViewState.stage = RenderCommandStage::View_ShadowMap;
		cascade.drawViewState.viewport = Rect(0, 0, cascade.mapSize, cascade.mapSize);
		cascade.drawViewState.viewId = reinterpret_cast<uint64_t>(view);
		cascade.drawViewState.requiredFlags = RenderFlags::ShadowCaster;
		cascade.drawViewState.materialVariant = MaterialVariant::DepthOnly;
		cascade.drawViewState.framebuffer = cascade.shadowMapFramebuffer;
		m_renderer->DrawView(cascade.drawViewState);

		// Transition and clear the shadow map.
		{
			std::shared_ptr<IGraphicsCommandBuffer> clearBuffer = m_renderer->RequestPrimaryBuffer();
			clearBuffer->Reset();
			clearBuffer->Begin();

			clearBuffer->TransitionResource(cascade.shadowMapImage, GraphicsAccessMask::ReadWrite);
			clearBuffer->Clear(cascade.shadowMapImage, Color::Black, 1.0f, 0.0f);

			clearBuffer->End();

			m_renderer->QueuePrimaryBuffer("Shadow Map Clear", RenderCommandStage::View_PreRender, clearBuffer, reinterpret_cast<uint64_t>(view));
		}

		// Transition back to reading format.
		{
			std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();
			drawBuffer->Reset();
			drawBuffer->Begin();

			drawBuffer->TransitionResource(cascade.shadowMapImage, GraphicsAccessMask::Read);

			drawBuffer->End();

			m_renderer->QueuePrimaryBuffer("Shadow Map Transition", RenderCommandStage::View_GBuffer, drawBuffer, reinterpret_cast<uint64_t>(view));
		}

	}, 1, "Shadow Map Cascade Building");

	// Update shadow mask rendering properties.
	Array<RenderPropertyImageSamplerValue> cascadeShadowMapsArray;
	Array<Matrix4> cascadeViewProjArray;
	Array<float> cascadeSplitDistanceArray;
	Array<float> cascadeRadiusArray;
	Array<float> cascadeMapSizeArray;

	for (int i = 0; i < Renderer::MaxShadowCascades; i++)
	{
		if (i < light->shadowMapCascades)
		{
			DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[i];

			cascadeViewProjArray.push_back(cascade.projectionMatrix * viewMatrix);
			cascadeSplitDistanceArray.push_back(cascade.splitMaxDistance);
			cascadeRadiusArray.push_back(cascade.worldRadius);
			cascadeMapSizeArray.push_back(cascade.mapSize);
			cascadeShadowMapsArray.push_back(RenderPropertyImageSamplerValue(cascade.shadowMapImageView, cascade.shadowMapSampler));
		}
		else
		{
			DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[0];

			cascadeViewProjArray.push_back(Matrix4::Identity);
			cascadeSplitDistanceArray.push_back(0.0f);
			cascadeRadiusArray.push_back(0.0f);
			cascadeMapSizeArray.push_back(0.0f);
			cascadeShadowMapsArray.push_back(RenderPropertyImageSamplerValue(cascade.shadowMapImageView, cascade.shadowMapSampler));
		}
	}

	light->shadowMaskProperties.Set(LightCascadeCountHash, light->shadowMapCascades);
	light->shadowMaskProperties.Set(LightViewPositionHash, Vector4(viewWorldLocation, 1.0f));
	light->shadowMaskProperties.Set(LightCascadeViewProjHash, cascadeViewProjArray);
	light->shadowMaskProperties.Set(LightCascadeSplitDistancesHash, cascadeSplitDistanceArray);
	light->shadowMaskProperties.Set(LightCascadeShadowMapsHash, cascadeShadowMapsArray);
	light->shadowMaskProperties.Set(LightCascadeRadiusHash, cascadeRadiusArray);	
	light->shadowMaskProperties.Set(LightWorldDirectionHash, Vector4(lightWorldDirection, 1.0f));
	light->shadowMaskProperties.Set(LightCascadeMapSizeHash, cascadeMapSizeArray);
	light->shadowMaskProperties.Set(LightShadowCascadeBlendFactorHash, (float)light->shadowMapCascadeBlendFactor);
	
	light->shadowMaskProperties.UpdateResources(m_graphics, m_logger);
}

void RenderDirectionalShadowMapSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<DirectionalLightComponent*>& lights,
	const Array<const TransformComponent*>& transforms)
{
	// Draw shadow map for each light.
	{
		ProfileScope scope(Color::Blue, "Update Directional Shadow Maps");

		std::shared_ptr<AspectCollection> viewCollection = world.GetAspectCollection(m_cameraViewAspectId);
		const Array<CameraViewComponent*>& viewComponents = viewCollection->GetEntityComponents<CameraViewComponent>();
		const Array<TransformComponent*>& viewTransformComponents = viewCollection->GetEntityComponents<TransformComponent>();

		for (int i = 0; i < viewComponents.size(); i++)
		{
			CameraViewComponent* view = viewComponents[i];

			for (size_t i = 0; i < entities.size(); i++)
			{
				DirectionalLightComponent* light = lights[i];
				const TransformComponent* transform = transforms[i];
				const TransformComponent* viewTransform = viewTransformComponents[i];

				if (light->isShadowCasting)
				{
					RenderLight(world, frameTime, light, view, transform, viewTransform);
				}
			}
		}
	}
}
