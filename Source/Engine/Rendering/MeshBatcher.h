#pragma once
#include "Pch.h"

#include "Engine/ECS/AspectCollection.h"
#include "Engine/ECS/Component.h"
#include "Engine/ECS/World.h"

class AspectCollection;
class Renderer;
struct MeshComponent;
struct TransformComponent;

struct MeshInstance
{
	const Array<std::shared_ptr<IGraphicsResourceSet>>* resourceSets;
	const std::shared_ptr<IGraphicsIndexBuffer>* indexBuffer;
	const std::shared_ptr<IGraphicsVertexBuffer>* vertexBuffer;
	int indexCount;
};

struct MaterialBatch
{
	Material* material;
	Array<MeshInstance> meshInstances;
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
	Array<MeshInstance*> meshInstances;
	bool inUse = false;
};

struct MeshBatcher
{
private:
	std::shared_ptr<AspectCollection> m_meshCollection;
	AspectId m_meshComponentAspectId = -1;

	Array<Dictionary<Material*, MaterialBatch>> m_asyncMaterialBatches;
	Array<MaterialRenderBatch*> m_materialRenderBatches;
	Array<MaterialRenderBatch*> m_finalRenderBatches;

	static const int MaxMeshesPerBatch = 500;

public:	
	typedef std::function<bool(Entity entity, const MeshComponent* mesh, const TransformComponent* transform)> FilterFunction_t;

	~MeshBatcher();

	void Batch(
		World& world, 
		const std::shared_ptr<Renderer>& renderer,
		const std::shared_ptr<Logger>& logger,
		const std::shared_ptr<IGraphics>& graphics,
		const Array<Entity>& entities, 
		MaterialVariant variant,
		RenderPropertyCollection* viewProperties,
		FilterFunction_t filter = nullptr);

	Array<MaterialRenderBatch*>& GetBatches();

};