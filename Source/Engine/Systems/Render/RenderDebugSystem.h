#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Camera/CameraViewComponent.h"
#include "Engine/ECS/System.h"

#include "Engine/Rendering/MeshBatcher.h"

#include "Engine/Types/Frustum.h"
#include "Engine/Types/Color.h"
#include "Engine/Types/Bounds.h"
#include "Engine/Types/OctTree.h"
#include "Engine/Types/OrientedBounds.h"

class Renderer;
class RenderView;
class IGraphicsCommandBuffer;

// Renders a debug line to the camera view.
struct DrawDebugLineMessage
{
	Vector3 start;
	Vector3 end;
	Color color;
};

// Renders a debug axis-aligned bounds to the camera view.
struct DrawDebugBoundsMessage
{
	Bounds bounds;
	Color color;
};

// Renders a debug oriented bounds to the camera view.
struct DrawDebugOrientedBoundsMessage
{
	OrientedBounds bounds;
	Color color;
};

// Renders a debug frustum to the camera view.
struct DrawDebugFrustumMessage
{
	Frustum frustum;
	Color color;
};

// Renders a debug sphere to the camera view.
struct DrawDebugSphereMessage
{
	Sphere sphere;
	Color color;
};

// Generates command buffers to render all debug primitives.
class RenderDebugSystem 
	: public System<CameraViewComponent, const TransformComponent>
{
private:
	struct RenderLine
	{
		Vector3 Start;
		Vector3 End;
		Color	LineColor;

		RenderLine() = default;

		RenderLine(const Vector3& start, const Vector3& end, const Color& color)
			: Start(start)
			, End(end)
			, LineColor(color)
		{
		}
	};

	struct DebugLineVertex
	{
		Vector3 position;
		Vector4 color;
	};

private:
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<IGraphics> m_graphics;
	Array<RenderLine> m_lines;

	std::shared_ptr<IGraphicsVertexBuffer> m_lineVertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> m_lineIndexBuffer;

	ResourcePtr<Material> m_debugLineMaterial;
	std::shared_ptr<MeshRenderState> m_debugLineMeshRenderState;

private:
	void RenderView(
		CameraViewComponent* views,
		const Array<DebugLineVertex>& lineVerts,
		const Array<uint32_t>& lineIndices);

public:
	RenderDebugSystem(
		std::shared_ptr<World> world, 
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<ResourceManager> resourceManager, 
		std::shared_ptr<IGraphics> graphics);

	virtual void Tick(
		World& world, 
		const FrameTime& frameTime, 
		const Array<Entity>& entities, 
		const Array<CameraViewComponent*>& views, 
		const Array<const TransformComponent*>& transforms);
};
