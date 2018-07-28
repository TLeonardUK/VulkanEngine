#include "Pch.h"

#include "Engine/Components/Camera/RenderCameraViewsSystem.h"
#include "Engine/Components/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/Transform/TransformSystem.h"
#include "Engine/Components/Transform/TransformUtils.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

RenderCameraViewsSystem::RenderCameraViewsSystem(std::shared_ptr<World> world, std::shared_ptr<Renderer> renderer)
	: m_renderer(renderer)
{
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();

	m_meshComponentAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
}

void RenderCameraViewsSystem::GatherVisibleMeshes(
	World& world, 
	const std::shared_ptr<RenderView>& renderView)
{
	// todo: grab mesh components that are relevant from oct-tree rather than going through all and culling?

	std::shared_ptr<AspectCollection> collection = world.GetAspectCollection(m_meshComponentAspectId);
	const Array<MeshComponent*>& meshComponents = collection->GetEntityComponents<MeshComponent>();
	const Array<TransformComponent*>& meshTransformComponents = collection->GetEntityComponents<TransformComponent>();
	size_t totalPossibleMeshes = 0;

	// Figure out maximum number of meshes that could be visible and make sure we have space for them.
	{
		ProfileScope scope(Color::Blue, "Get total meshes");

		for (size_t i = 0; i < meshComponents.size(); i++)
		{
			MeshComponent* meshComponent = meshComponents[i];

			std::shared_ptr<Model> model = meshComponent->model.Get();
			const Array<std::shared_ptr<Mesh>>& meshes = model->GetMeshes();

			totalPossibleMeshes += meshes.size();
		}
	}

	{
		ProfileScope scope(Color::Blue, "Resizing mesh list");

		renderView->Meshes.resize(totalPossibleMeshes);
	}

	std::atomic<int> totalVisibleMeshes = 0;
	size_t count = meshComponents.size();

	// Figure out which meshes are actually visible.
	ParallelFor(static_cast<int>(meshComponents.size()), [&](int i)
	{
		MeshComponent* meshComponent = meshComponents[i];
		TransformComponent* meshTransformComponent = meshTransformComponents[i];

		std::shared_ptr<Model> model = meshComponent->model.Get();
		const Array<std::shared_ptr<Mesh>>& meshes = model->GetMeshes();

		meshComponent->renderData.resize(meshes.size());

		ParallelFor(static_cast<int>(meshes.size()), [&](int j)
		{
			if (renderView->Frustum.Intersects(meshComponent->bounds[j]) != FrustumIntersection::Outside)
			{
				int meshIndex = totalVisibleMeshes++;

				RenderMesh& renderMesh = renderView->Meshes[meshIndex];
				renderMesh.MeshComponent = meshComponent;
				renderMesh.MeshIndex = j;
				renderMesh.Mesh = meshes[j];

				// Apply transform properties for rendering..
				meshComponent->properties.Set(ModelMatrixHash, meshTransformComponent->localToWorld);

				// Draw mesh bounds if enabled.
				if (m_renderer->IsDrawBoundsEnabled())
				{
					DrawDebugOrientedBoundsMessage boundsDrawMsg;
					boundsDrawMsg.bounds = meshComponent->bounds[j];
					boundsDrawMsg.color = Color(0.0f, 0.5f, 1.0f, 1.0f);
					world.QueueMessage(boundsDrawMsg);
				}
			}
		}, 16, "Sub-Mesh Frustum Culling");

	}, 16, "Frustum Culling");

	// Resize to number of meshes we actually found.
	renderView->Meshes.resize(totalVisibleMeshes);
}

void RenderCameraViewsSystem::TickView(
	World& world,
	const FrameTime& frameTime,
	CameraViewComponent* view,
	const TransformComponent* transform,
	const std::shared_ptr<RenderView>& renderView)
{
	Vector3 worldLocation = TransformUtils::GetWorldLocation(transform);

	int swapWidth = m_renderer->GetSwapChainWidth();
	int swapHeight = m_renderer->GetSwapChainHeight();

	Vector3 forward = TransformUtils::GetForwardVector(transform);

	renderView->Viewport.x = view->viewport.x * swapWidth;
	renderView->Viewport.y = view->viewport.y * swapHeight;
	renderView->Viewport.width = view->viewport.width * swapWidth;
	renderView->Viewport.height = view->viewport.height * swapHeight;
	renderView->ViewMatrix = Matrix4::LookAt(worldLocation, worldLocation + TransformUtils::GetForwardVector(transform), Vector3::Up); // negative to correct for vulkans flipped y.
	renderView->ProjectionMatrix = Matrix4::Perspective(Math::Radians(view->fov), renderView->Viewport.width / renderView->Viewport.height, view->depthMin, view->depthMax);
	renderView->CameraPosition = worldLocation;

	if (!m_renderer->IsRenderingFrozen())
	{
		view->frustum = Frustum(renderView->ProjectionMatrix * renderView->ViewMatrix);
	}

	renderView->Frustum = view->frustum;
	renderView->Lines.clear();
	renderView->Meshes.clear();

	if (m_renderer->IsDrawBoundsEnabled())
	{
		DrawDebugFrustumMessage frusumDrawMsg;
		frusumDrawMsg.frustum = renderView->Frustum;
		frusumDrawMsg.color = Color(0.0f, 1.0f, 0.0f, 1.0f);
		world.QueueMessage(frusumDrawMsg);
	}

	// Gather all meshes visible within the view frustum.
	GatherVisibleMeshes(world, renderView);
}

void RenderCameraViewsSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<CameraViewComponent*>& views,
	const Array<const TransformComponent*>& transforms)
{
	Array<std::shared_ptr<RenderView>> renderViews;
	
	// Consume all relevant messages.

	// Consume all relevant messages.
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

	// Draw visible meshes for each view.
	for (size_t i = 0; i < entities.size(); i++)
	{
		CameraViewComponent* view = views[i];
		const TransformComponent* transform = transforms[i];

		std::shared_ptr<RenderView> renderView = m_renderer->QueueView();
		renderViews.push_back(renderView);

		TickView(world, frameTime, view, transform, renderView);
	}

	// Draw debug lines.
	for (auto& message : world.ConsumeMessages<DrawDebugLineMessage>())
	{
		RenderLine line(message.start, message.end, message.color);

		for (size_t i = 0; i < renderViews.size(); i++)
		{
			std::shared_ptr<RenderView>& renderView = renderViews[i];
			renderView->Lines.push_back(line);
		}
	}

	// Draw frustum.
	for (auto& message : world.ConsumeMessages<DrawDebugFrustumMessage>())
	{
		Vector3 corners[(int)FrustumCorners::Count];
		message.frustum.GetCorners(corners);

		RenderLine lines[12] = {
			RenderLine(corners[(int)FrustumCorners::FarTopLeft], corners[(int)FrustumCorners::FarTopRight], message.color),
			RenderLine(corners[(int)FrustumCorners::FarBottomLeft], corners[(int)FrustumCorners::FarBottomRight], message.color),
			RenderLine(corners[(int)FrustumCorners::FarTopLeft], corners[(int)FrustumCorners::FarBottomLeft], message.color),
			RenderLine(corners[(int)FrustumCorners::FarTopRight], corners[(int)FrustumCorners::FarBottomRight], message.color),

			RenderLine(corners[(int)FrustumCorners::NearTopLeft], corners[(int)FrustumCorners::NearTopRight], message.color),
			RenderLine(corners[(int)FrustumCorners::NearBottomLeft], corners[(int)FrustumCorners::NearBottomRight], message.color),
			RenderLine(corners[(int)FrustumCorners::NearTopLeft], corners[(int)FrustumCorners::NearBottomLeft], message.color),
			RenderLine(corners[(int)FrustumCorners::NearTopRight], corners[(int)FrustumCorners::NearBottomRight], message.color),

			RenderLine(corners[(int)FrustumCorners::NearTopLeft], corners[(int)FrustumCorners::FarTopLeft], message.color),
			RenderLine(corners[(int)FrustumCorners::NearTopRight], corners[(int)FrustumCorners::FarTopRight], message.color),
			RenderLine(corners[(int)FrustumCorners::NearBottomLeft], corners[(int)FrustumCorners::FarBottomLeft], message.color),
			RenderLine(corners[(int)FrustumCorners::NearBottomRight], corners[(int)FrustumCorners::FarBottomRight], message.color),
		};

		for (size_t i = 0; i < renderViews.size(); i++)
		{
			std::shared_ptr<RenderView>& renderView = renderViews[i];
			renderView->Lines.reserve(renderView->Lines.size() + 12);
			for (int j = 0; j < 12; j++)
			{
				renderView->Lines.push_back(lines[j]);
			}
		}
	}

	// Draw debug bounds.
	for (auto& message : world.ConsumeMessages<DrawDebugBoundsMessage>())
	{
		Vector3 corners[(int)BoundsCorners::Count];
		message.bounds.GetCorners(corners);

		RenderLine lines[12] = {
			RenderLine(corners[(int)BoundsCorners::BackTopLeft], corners[(int)BoundsCorners::BackTopRight], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontTopLeft], corners[(int)BoundsCorners::FrontTopRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopLeft], corners[(int)BoundsCorners::FrontTopLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopRight], corners[(int)BoundsCorners::FrontTopRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackBottomLeft], corners[(int)BoundsCorners::BackBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontBottomLeft], corners[(int)BoundsCorners::FrontBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackBottomLeft], corners[(int)BoundsCorners::FrontBottomLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::BackBottomRight], corners[(int)BoundsCorners::FrontBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopLeft], corners[(int)BoundsCorners::BackBottomLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopRight], corners[(int)BoundsCorners::BackBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontTopLeft], corners[(int)BoundsCorners::FrontBottomLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontTopRight], corners[(int)BoundsCorners::FrontBottomRight], message.color)
		};

		for (size_t i = 0; i < renderViews.size(); i++)
		{
			std::shared_ptr<RenderView>& renderView = renderViews[i];
			renderView->Lines.reserve(renderView->Lines.size() + 12);
			for (int j = 0; j < 12; j++)
			{
				renderView->Lines.push_back(lines[j]);
			}
		}
	}

	// Draw debug orinted bounds.
	for (auto& message : world.ConsumeMessages<DrawDebugOrientedBoundsMessage>())
	{
		Vector3 corners[(int)BoundsCorners::Count];
		message.bounds.GetCorners(corners);

		RenderLine lines[12] = {
			RenderLine(corners[(int)BoundsCorners::BackTopLeft], corners[(int)BoundsCorners::BackTopRight], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontTopLeft], corners[(int)BoundsCorners::FrontTopRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopLeft], corners[(int)BoundsCorners::FrontTopLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopRight], corners[(int)BoundsCorners::FrontTopRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackBottomLeft], corners[(int)BoundsCorners::BackBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontBottomLeft], corners[(int)BoundsCorners::FrontBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackBottomLeft], corners[(int)BoundsCorners::FrontBottomLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::BackBottomRight], corners[(int)BoundsCorners::FrontBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopLeft], corners[(int)BoundsCorners::BackBottomLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::BackTopRight], corners[(int)BoundsCorners::BackBottomRight], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontTopLeft], corners[(int)BoundsCorners::FrontBottomLeft], message.color),
			RenderLine(corners[(int)BoundsCorners::FrontTopRight], corners[(int)BoundsCorners::FrontBottomRight], message.color)
		};

		for (size_t i = 0; i < renderViews.size(); i++)
		{
			std::shared_ptr<RenderView>& renderView = renderViews[i];
			renderView->Lines.reserve(renderView->Lines.size() + 12);
			for (int j = 0; j < 12; j++)
			{
				renderView->Lines.push_back(lines[j]);
			}
		}
	}
}
