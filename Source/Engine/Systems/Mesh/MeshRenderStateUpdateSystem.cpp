#include "Pch.h"

#include "Engine/Components/Mesh/ModelComponent.h"

#include "Engine/Systems/Mesh/MeshRenderStateUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"

#include "Engine/Rendering/Renderer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

MeshRenderStateUpdateSystem::MeshRenderStateUpdateSystem(
	std::shared_ptr<IGraphics> graphics,
	std::shared_ptr<Logger> logger)
	: m_graphics(graphics)
	, m_logger(logger)
{
	AddPredecessor<TransformSystem>();
}

void MeshRenderStateUpdateSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<MeshComponent*>& meshes,
	const Array<const TransformComponent*>& transforms)
{
	ParallelFor(static_cast<int>(entities.size()), [&](int i)
	{
		const TransformComponent* transform = transforms[i];
		MeshComponent* mesh = meshes[i];
		
		if (mesh->lastTransformVersion != transform->version)
		{
			// Calculate render flags for mesh.
			RenderFlags flags = RenderFlags::None;

			std::shared_ptr<Shader> shader = mesh->mesh->GetMaterial().Get()->GetShader().Get();
			if (shader->GetProperties().ShadowReciever)
			{
				flags = (RenderFlags)((int)flags | (int)RenderFlags::ShadowReciever);
			}

			// Apply properties.
			mesh->properties.Set(ModelMatrixHash, transform->localToWorld);
			mesh->properties.Set(RenderFlagsHash, (int)flags);
			mesh->properties.UpdateResources(m_graphics, m_logger);

			mesh->lastTransformVersion = transform->version;
		}
	}, 16, "Render State Update");
}