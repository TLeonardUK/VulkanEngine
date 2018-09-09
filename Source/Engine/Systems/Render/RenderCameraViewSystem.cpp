#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Mesh/MeshRenderStateUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Types/OctTree.h"

RenderCameraViewSystem::RenderCameraViewSystem(
	std::shared_ptr<World> world,
	std::shared_ptr<IGraphics> graphics,
	std::shared_ptr<Logger> logger,
	std::shared_ptr<Renderer> renderer)
	: m_renderer(renderer)
	, m_logger(logger)
	, m_graphics(graphics)
{
	AddPredecessor<MeshRenderStateUpdateSystem>();
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();

	m_meshComponentAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
}

void RenderCameraViewSystem::TickView(
	World& world,
	const FrameTime& frameTime,
	CameraViewComponent* view,
	const TransformComponent* transform)
{
	SpatialIndexSystem* spatialSystem = world.GetSystem<SpatialIndexSystem>();

	Vector3 worldLocation = TransformUtils::GetWorldLocation(transform);
	Vector3 forward = TransformUtils::GetForwardVector(transform);

	int swapWidth = m_renderer->GetSwapChainWidth();
	int swapHeight = m_renderer->GetSwapChainHeight();

	Rect viewport(
		0.0f,
		0.0f,
		swapWidth,
		swapHeight
	);

	view->viewMatrix = Matrix4::LookAt(worldLocation, worldLocation + TransformUtils::GetForwardVector(transform), Vector3::Up);
	view->projectionMatrix = Matrix4::Perspective(Math::Radians(view->fov), viewport.width / viewport.height, view->depthMin, view->depthMax);
	Vector3 cameraPosition = worldLocation;

	if (!m_renderer->IsRenderingFrozen())
	{
		view->frustum = Frustum(view->projectionMatrix * view->viewMatrix);
		view->viewSpaceFrustum = Frustum(view->projectionMatrix);
	}

	if (m_renderer->IsDrawBoundsEnabled())
	{
		DrawDebugFrustumMessage frusumDrawMsg;
		frusumDrawMsg.frustum = view->frustum;
		frusumDrawMsg.color = Color::Green;
		world.QueueMessage(frusumDrawMsg);

		DrawDebugBoundsMessage boundsMsg;
		boundsMsg.color = Color::PureRed;
		boundsMsg.bounds = Bounds::FromCenterAndExtents(view->frustum.GetOrigin(), Vector3(5.0f, 5.0f, 5.0f));
		world.QueueMessage(boundsMsg);
	}

	// Update global properties.
	view->viewProperties.Set(ViewMatrixHash, view->viewMatrix);
	view->viewProperties.Set(ProjectionMatrixHash, view->projectionMatrix);
	view->viewProperties.Set(CameraPositionHash, cameraPosition);
	view->viewProperties.UpdateResources(m_graphics, m_logger);

	// Draw view to frame buffer.
	m_drawViewState.world = &world;
	m_drawViewState.frustum = view->frustum;
	m_drawViewState.viewProperties = &view->viewProperties;
	m_drawViewState.name = "Camera View";
	m_drawViewState.stage = RenderCommandStage::View_GBuffer;
	m_drawViewState.viewport = viewport;
	m_drawViewState.viewId = reinterpret_cast<uint64_t>(view);
	m_renderer->DrawView(m_drawViewState);

	// Transition buffer at start of rendering.
	{
		std::shared_ptr<IGraphicsCommandBuffer> buffer = m_renderer->RequestPrimaryBuffer();

		buffer->Reset();
		buffer->Begin();

		// Transition all images so they can be written to.
		buffer->TransitionResource(m_renderer->GetDepthImage(), GraphicsAccessMask::ReadWrite);
		buffer->TransitionResource(m_renderer->GetShadowMaskImage(), GraphicsAccessMask::Write);
		buffer->TransitionResource(m_renderer->GetLightAccumulationImage(), GraphicsAccessMask::Write);
		for (int i = 0; i < Renderer::GBufferImageCount; i++)
		{
			buffer->TransitionResource(m_renderer->GetGBufferImage(i), GraphicsAccessMask::Write);
		}

		// Clear all buffers we will be rendering to.
		buffer->Clear(m_renderer->GetDepthImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
		buffer->Clear(m_renderer->GetShadowMaskImage(), Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
		buffer->Clear(m_renderer->GetLightAccumulationImage(), Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f);

		// DBEUG DEBUG DEBUG DEBUG
		buffer->Clear(m_renderer->GetLightAccumulationImage(), Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
		// DBEUG DEBUG DEBUG DEBUG

		// Clear G-Buffer
		m_renderer->DrawFullScreenQuad(buffer, m_renderer->GetClearGBufferMaterial().Get(), &m_renderer->GetClearGBufferRenderState(), nullptr, nullptr);

		buffer->End();

		m_renderer->QueuePrimaryBuffer("Camera View Pre Transitions", RenderCommandStage::View_PreRender, buffer, reinterpret_cast<uint64_t>(view));
	}

	// Transition/resolve buffer at end of rendering.
	{
		std::shared_ptr<IGraphicsCommandBuffer> buffer = m_renderer->RequestPrimaryBuffer();

		buffer->Reset();
		buffer->Begin();

		// Transition all images so they can be read from.
		buffer->TransitionResource(m_renderer->GetShadowMaskImage(), GraphicsAccessMask::Read);
		buffer->TransitionResource(m_renderer->GetLightAccumulationImage(), GraphicsAccessMask::Read);
		for (int i = 0; i < Renderer::GBufferImageCount; i++)
		{
			buffer->TransitionResource(m_renderer->GetGBufferImage(i), GraphicsAccessMask::Read);
		}

		// Resolve to framebuffer.
		// todo: support resolving to texture etc.
		m_renderer->DrawFullScreenQuad(
			buffer, 
			m_renderer->GetResolveToSwapChainMaterial().Get(), 
			&m_renderer->GetResolveToSwapChainRenderState(), 
			nullptr, 
			nullptr,
			view->viewport,
			view->viewport);

		buffer->End();

		m_renderer->QueuePrimaryBuffer("Camera View Post Transitions", RenderCommandStage::View_PostRender, buffer, reinterpret_cast<uint64_t>(view));
	}
}

void RenderCameraViewSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<CameraViewComponent*>& views,
	const Array<const TransformComponent*>& transforms)
{
	// Consume all relevant messages.
	{
		ProfileScope scope(ProfileColors::Draw, "Tick Messages");

		for (auto& message : world.ConsumeMessages<SetCameraViewProjectionMessage>())
		{
			CameraViewComponent* component = message.component.Get(world);
			if (component != nullptr)
			{
				component->viewport = message.viewport;
				component->depthMin = message.depthMin;
				component->depthMax = message.depthMax;
				component->fov = message.fov;
			}
		}
	}

	// Draw visible meshes for each view.
	{
		ProfileScope scope(ProfileColors::Draw, "Tick Views");

		for (size_t i = 0; i < entities.size(); i++)
		{
			CameraViewComponent* view = views[i];
			const TransformComponent* transform = transforms[i];

			TickView(world, frameTime, view, transform);
		}
	}
}
