#include "Pch.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Rendering/MeshRenderState.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Utilities/Statistic.h"

MeshRenderState::MeshRenderState(
	const std::shared_ptr<Logger>& logger,
	const std::shared_ptr<Renderer>& renderer,
	const std::shared_ptr<IGraphics>& graphics)
	: m_logger(logger)
	, m_renderer(renderer)
	, m_graphics(graphics)
{
}

MeshRenderState::~MeshRenderState()
{
	for (auto setsPair : m_sets)
	{
		delete setsPair.second;
	}
}

void MeshRenderState::RecreateHeirarchyResourceSet(
	RenderPropertyHeirarchy* heirarchy,
	HeirarchyResourceSetData* resourceSet)
{
	const Array<MaterialResourceSet>& resourceSets = resourceSet->lastKnownMaterial->GetResourceSets();
	const Array<ShaderBinding>& bindings = resourceSet->lastKnownMaterial->GetShader().Get()->GetBindings();

	resourceSet->sets.clear();
	resourceSet->sets.reserve(resourceSets.size());

	for (const MaterialResourceSet& set : resourceSets)
	{
		RenderPropertyCollection* collection = heirarchy->Get(set.frequency);

		// todo: do update in here if resource-set has changed.
		resourceSet->sets.push_back(collection->GetResourceSet(m_renderer, m_graphics, m_logger, set)); 
	}
}

const Array<std::shared_ptr<IGraphicsResourceSet>>& MeshRenderState::UpdateAndGetResourceSets(
	const std::shared_ptr<Material>& material,
	RenderPropertyHeirarchy* heirarchy)
{
	ScopeLock lock(m_setsMutex); // todo: need to remove this fucker.

	HeirarchyResourceSetData* resourceSet;

	uint64_t hashCode = heirarchy->GetHashCode();
	bool bIsNew = false;

	// todo: purge old set instances (do we care? There won't be many and don't take much space?)
	// what if we have 1000 lights in a scene? this could blow up fast?
	// todo: something else should cache this data higher up? 
	// only store last x in array? Can just shift through and grab whats important. New heirarchy values
	// just do an atomic swap of entry they want? no locking required.

	// Get state stored for this heirarchy combo.
	auto setsIter = m_sets.find(hashCode);
	if (setsIter == m_sets.end())
	{
		resourceSet = new HeirarchyResourceSetData();
		bIsNew = true;
		m_sets.emplace(hashCode, resourceSet);
	}
	else
	{
		resourceSet = setsIter->second;
	}

	// If anything has changed that could cause resource-sets to be invalid, then re-generate.
	if (bIsNew || 
		&*material != &*resourceSet->lastKnownMaterial)
	{
		resourceSet->lastKnownMaterial = material;

		RecreateHeirarchyResourceSet(heirarchy, resourceSet);
	}

	return resourceSet->sets;
}