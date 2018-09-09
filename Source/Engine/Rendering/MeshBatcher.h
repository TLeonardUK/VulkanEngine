#pragma once
#include "Pch.h"

#include "Engine/ECS/AspectCollection.h"
#include "Engine/ECS/Component.h"
#include "Engine/ECS/World.h"

#include "Engine/Resources/Types/Material.h"
#include "Engine/Rendering/RendererEnums.h"

#include "Engine/Types/StackAllocator.h"

class AspectCollection;
class Renderer;
struct MeshComponent;
struct TransformComponent;
class IGraphicsIndexBuffer;
class IGraphicsVertexBuffer;
class IGraphicsResourceSet;

struct MeshInstance
{
	const Array<std::shared_ptr<IGraphicsResourceSet>>* resourceSets;
	const std::shared_ptr<IGraphicsIndexBuffer>* indexBuffer;
	const std::shared_ptr<IGraphicsVertexBuffer>* vertexBuffer;
	int indexCount;

	MeshInstance* nextMesh;
};

struct MaterialBatch
{
	Material* material;
	MeshInstance* firstMesh;
	int meshInstanceCount;
};

struct MaterialRenderSubBatch
{
	MaterialBatch* batch;
	int startIndex;
	int endIndex;
};

struct MaterialRenderBatch
{
	Material* material;
	MeshInstance* firstMesh;
	int meshInstanceCount;
	bool inUse;
};

struct MeshBatcher
{
private:
	std::shared_ptr<AspectCollection> m_meshCollection;
	AspectId m_meshComponentAspectId = -1;

	Array<Dictionary<Material*, MaterialBatch>> m_asyncMaterialBatches;
	Array<MaterialRenderBatch*> m_materialRenderBatches;
	Array<MaterialRenderBatch*> m_finalRenderBatches;
	
	StackAllocator<MeshInstance> m_meshInstanceAllocator;

	static const int MaxMeshesPerBatch = 500;

public:	
	~MeshBatcher();

	void Batch(
		World& world, 
		const std::shared_ptr<Renderer>& renderer,
		const std::shared_ptr<Logger>& logger,
		const std::shared_ptr<IGraphics>& graphics,
		const Array<Entity>& entities, 
		MaterialVariant variant,
		RenderPropertyCollection* viewProperties,
		RenderFlags requiredFlags = RenderFlags::None,
		RenderFlags excludedFlags = RenderFlags::None);

	Array<MaterialRenderBatch*>& GetBatches();

};