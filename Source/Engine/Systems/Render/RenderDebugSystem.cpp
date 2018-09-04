#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Rendering/RenderPropertyHeirarchy.h"

#include "Engine/Graphics/Graphics.h"

#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Types/OctTree.h"

RenderDebugSystem::RenderDebugSystem(
	std::shared_ptr<World> world, 
	std::shared_ptr<Renderer> renderer, 
	std::shared_ptr<ResourceManager> resourceManager, 
	std::shared_ptr<IGraphics> graphics
)
	: m_renderer(renderer)
	, m_graphics(graphics)
{
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();
	AddPredecessor<RenderCameraViewSystem>();

	m_debugLineMaterial = resourceManager->Load<Material>("Engine/Materials/debug_line.json");
	m_debugLineMaterial.WaitUntilLoaded();

	renderer->CreateMeshRenderState(&m_debugLineMeshRenderState);
}

void RenderDebugSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<CameraViewComponent*>& views,
	const Array<const TransformComponent*>& transforms)
{
	Array<DrawDebugLineMessage> debugLineMessages = world.ConsumeMessages<DrawDebugLineMessage>();
	Array<DrawDebugFrustumMessage> debugFrustumMessages = world.ConsumeMessages<DrawDebugFrustumMessage>();
	Array<DrawDebugSphereMessage> debugSphereMessages = world.ConsumeMessages<DrawDebugSphereMessage>();
	Array<DrawDebugBoundsMessage> debugBoundsMessages = world.ConsumeMessages<DrawDebugBoundsMessage>();
	Array<DrawDebugOrientedBoundsMessage> debugOrientedBoundsMessages = world.ConsumeMessages<DrawDebugOrientedBoundsMessage>();

	m_lines.clear();
	m_lines.reserve(
		(debugLineMessages.size() * 2) +
		(debugFrustumMessages.size() * 12) +
		(debugBoundsMessages.size() * 12) +
		(debugOrientedBoundsMessages.size() * 12)
	);

	// Draw debug lines.
	for (auto& message : debugLineMessages)
	{
		RenderLine line(message.start, message.end, message.color);
		m_lines.push_back(line);
	}

	// Draw frustum.
	for (auto& message : debugFrustumMessages)
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

		for (int j = 0; j < 12; j++)
		{
			m_lines.push_back(lines[j]);
		}
	}

	// Draw debug bounds.
	for (auto& message : debugBoundsMessages)
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

		for (int j = 0; j < 12; j++)
		{
			m_lines.push_back(lines[j]);
		}
	}

	// Draw debug orinted bounds.
	for (auto& message : debugOrientedBoundsMessages)
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

		for (int j = 0; j < 12; j++)
		{
			m_lines.push_back(lines[j]);
		}
	}

	// Draw debug sphere.
	for (auto& message : debugSphereMessages)
	{
		const float PI = 3.141592f;
		const int gradation = 20;

		float radius = message.sphere.radius;
		Vector3 origin = message.sphere.origin;

		for (float alpha = 0.0; alpha < Math::Pi; alpha += PI / gradation)
		{
			for (float beta = 0.0; beta < 2.01 * Math::Pi; beta += PI / gradation)
			{
				Vector3 start(
					radius * cos(beta) * sin(alpha),
					radius * sin(beta) * sin(alpha),
					radius * cos(alpha)
				);

				Vector3 end(
					radius * cos(beta) * sin(alpha + PI / gradation),
					radius * sin(beta) * sin(alpha + PI / gradation),
					radius * cos(alpha + PI / gradation)
				);

				m_lines.push_back(RenderLine(origin + start, origin + end, message.color));
			}
		}
	}

	// Generate indices.
	Array<uint32_t> lineIndices;
	Array<DebugLineVertex> lineVerts;
	lineVerts.resize(m_lines.size() * 2);
	lineIndices.resize(m_lines.size() * 2);

	for (size_t i = 0; i < m_lines.size(); i++)
	{
		RenderLine& line = m_lines[i];

		lineVerts[(i * 2) + 0].position = line.Start;
		lineVerts[(i * 2) + 0].color = line.LineColor.ToVector();
		lineVerts[(i * 2) + 1].position = line.End;
		lineVerts[(i * 2) + 1].color = line.LineColor.ToVector();

		lineIndices[(i * 2) + 0] = static_cast<uint32_t>((i * 2) + 0);
		lineIndices[(i * 2) + 1] = static_cast<uint32_t>((i * 2) + 1);
	}

	if (lineVerts.size() == 0)
	{
		return;
	}
	
	if (m_lineVertexBuffer == nullptr || lineVerts.size() > m_lineVertexBuffer->GetCapacity())
	{
		VertexBufferBindingDescription description;
		m_debugLineMaterial.Get()->GetVertexBufferFormat(description);

		m_lineVertexBuffer = m_graphics->CreateVertexBuffer("Debug Line Vertex Buffer", description, static_cast<int>(lineVerts.size()));
	}

	if (m_lineIndexBuffer == nullptr || lineIndices.size() > m_lineIndexBuffer->GetCapacity())
	{
		m_lineIndexBuffer = m_graphics->CreateIndexBuffer("Debug Line Index Buffer", sizeof(uint32_t), static_cast<int>(lineIndices.size()));
	}

	m_lineVertexBuffer->Stage(lineVerts.data(), 0, static_cast<int>(lineVerts.size()) * sizeof(DebugLineVertex));
	m_lineIndexBuffer->Stage(lineIndices.data(), 0, static_cast<int>(lineIndices.size()) * sizeof(uint32_t));

	// Draw lines on each view.
	for (int i = 0; i < views.size(); i++)
	{
		CameraViewComponent* view = views[i];
		RenderView(view, lineVerts, lineIndices);
	}
}

void RenderDebugSystem::RenderView(
	CameraViewComponent* view,
	const Array<DebugLineVertex>& lineVerts,
	const Array<uint32_t>& lineIndices)
{
	ProfileScope scope(ProfileColors::Draw, "RenderDebugSystem::RenderView");

	int swapWidth = m_renderer->GetSwapChainWidth();
	int swapHeight = m_renderer->GetSwapChainHeight();

	Rect viewport(
		0.0f,
		0.0f,
		swapWidth,
		swapHeight
	);

	// Update global properties.
	view->viewProperties.Set(ViewMatrixHash, view->viewMatrix);
	view->viewProperties.Set(ProjectionMatrixHash, view->projectionMatrix);

	std::shared_ptr<IGraphicsCommandBuffer> buffer = m_renderer->RequestPrimaryBuffer();
	buffer->Reset();
	buffer->Begin();

	buffer->Upload(m_lineVertexBuffer);
	buffer->Upload(m_lineIndexBuffer);

	// todo: set projection/view matrices.

	// Get material.
	std::shared_ptr<Material> material = m_debugLineMaterial.Get();
	material->UpdateResources();

	// Create render property heirarchy.
	RenderPropertyHeirarchy renderHeirarchy;
	renderHeirarchy.Set(GraphicsBindingFrequency::Global, &m_renderer->GetGlobalRenderProperties());
	renderHeirarchy.Set(GraphicsBindingFrequency::View, &view->viewProperties);
	renderHeirarchy.Set(GraphicsBindingFrequency::Material, &material->GetProperties());
	renderHeirarchy.Set(GraphicsBindingFrequency::Mesh, nullptr);

	const Array<std::shared_ptr<IGraphicsResourceSet>>& resourceSets = m_debugLineMeshRenderState->UpdateAndGetResourceSets(material, &renderHeirarchy);

	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
	buffer->SetScissor(
		static_cast<int>(viewport.x),
		static_cast<int>(viewport.y),
		static_cast<int>(viewport.width),
		static_cast<int>(viewport.height)
	);
	buffer->SetViewport(
		static_cast<int>(viewport.x),
		static_cast<int>(viewport.y),
		static_cast<int>(viewport.width),
		static_cast<int>(viewport.height)
	);

	buffer->SetIndexBuffer(m_lineIndexBuffer);
	buffer->SetVertexBuffer(m_lineVertexBuffer);
	buffer->SetResourceSets(resourceSets.data(), (int)resourceSets.size());

	buffer->DrawIndexedElements(static_cast<int>(lineVerts.size()), 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();

	buffer->End();
	m_renderer->QueuePrimaryBuffer("Debug Primitives", RenderCommandStage::View_Debug, buffer, reinterpret_cast<uint64_t>(view));
}