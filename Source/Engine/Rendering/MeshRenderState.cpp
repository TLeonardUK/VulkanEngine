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

		// Instantiate resource-set if neccessary.
		collection->GetResourceSet(m_renderer, m_graphics, set);
	}

	for (const MaterialResourceSet& set : resourceSets)
	{
		RenderPropertyCollection* collection = heirarchy->Get(set.frequency);

		// Update the heirarchy resources if required.
		collection->UpdateResources(m_graphics, m_logger);

		// Store our actual resource-set.
		resourceSet->sets.push_back(collection->GetResourceSet(m_renderer, m_graphics, set));
	}
}

const Array<std::shared_ptr<IGraphicsResourceSet>>& MeshRenderState::UpdateAndGetResourceSets(
	const std::shared_ptr<Material>& material,
	RenderPropertyHeirarchy* heirarchy)
{
	ScopeLock lock(m_setsMutex);

	HeirarchyResourceSetData* resourceSet;

	uint64_t hashCode = heirarchy->GetHashCode();
	bool bIsNew = false;

	// todo: purge old set instances (do we care? There won't be many and don't take much space?)

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

	// If material has changed, heirarchy version has changed or this
	// is a heirarchy we haven't encountered before, we need to regenerate 
	// our list of resource-sets. 
	//
	// TODO: This is super-shitty as the view heirarchy version updates each version,
	// resulting in us re-running this every frame -_-. Update each heirarchy level at appropriate place 
	// instead of here.
	if (bIsNew || 
		&*material != &*resourceSet->lastKnownMaterial ||
		heirarchy->GetVersion() != resourceSet->lastHeirarchyVersion)
	{
		resourceSet->lastKnownMaterial = material;

		RecreateHeirarchyResourceSet(heirarchy, resourceSet);
	}

	return resourceSet->sets;




	// should we take in a task-specific buffer we cache this data in? Save a mutex lock.

	// need to cache unique values for each heirarchy combination.
	// todo: purge out values if heirarchies are removed (is this really neccessary, meshes are not going to render with different heirarchies often)?
	//ScopeLock lock(m_resourceMutex); // todo: not required.
	
	// Todo: ech this is gonna be slow.
	//const Array<MaterialResourceSet>& resourceSets = material->GetResourceSets();
	//const Array<ShaderBinding>& bindings = material->GetShader().Get()->GetBindings();

	// ############################################
	// TODO NOW: MaterialResourceSet's are currently shared. This explodes if multiple threads are tryhing
	// to update the sames ones at the same time. Also won't meshes with seperate matrices etc.
	// ############################################

	//output->reserve(resourceSets.size());
	//output->clear();

	// todo: all mutex frequency ubo's/resource-set's neext to be updated
	//       inside a mutex (they should only update once per frame though).

	// Get all resource sets.
	/*for (const MaterialResourceSet& set : resourceSets)
	{
		RenderPropertyCollection* collection = heirarchy->Get(set.frequency);

		// This is slow as shit, fix nao.
		collection->GetResourceSet(m_graphics, set); // once to create if neccessary.
		collection->UpdateResources(m_graphics, m_logger);
		output->push_back(collection->GetResourceSet(m_graphics, set));
	}*/

	/*if (&*material != &*m_lastKnownMaterial)
	{
		m_lastKnownMaterial = material;

		Recreate(heirarchy);
	}*/
}

/*	ScopeLock lock(m_resourceMutex); // todo: not required.

	const Array<MaterialResourceSet>& resourceSets = m_lastKnownMaterial->GetResourceSets();
	const Array<ShaderBinding>& bindings = m_lastKnownMaterial->GetShader().Get()->GetBindings();
	
	m_resourceSets.clear();

	// todo: all mutex frequency ubo's/resource-set's neext to be updated
	//       inside a mutex (they should only update once per frame though).

	// Get all resource sets.
	for (const MaterialResourceSet& set : resourceSets)
	{
		m_resourceSets.push_back(heirarchy->Get(set.frequency)->GetResourceSet(set));
	}
*/