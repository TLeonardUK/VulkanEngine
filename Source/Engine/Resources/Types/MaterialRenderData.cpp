#include "Pch.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Resources/Types/MaterialRenderData.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Utilities/Statistic.h"

MaterialRenderData::MaterialRenderData(
	const std::shared_ptr<Logger>& logger,
	const std::shared_ptr<Renderer>& renderer,
	const std::shared_ptr<IGraphics>& graphics)
	: m_logger(logger)
	, m_renderer(renderer)
	, m_graphics(graphics)
{
}

void MaterialRenderData::Update(
	const std::shared_ptr<Material>& material,
	MaterialPropertyCollection* meshPropertiesCollection)
{
	bool regenerate = true;

	if (&*material != &*m_lastKnownMaterial)
	{
		m_lastKnownMaterial = material;
		m_lastMeshPropertiesVersion = -1;
		m_lastMaterialPropertiesVersion = -1;

		Recreate();
	}

	int meshPropertiesVersion = (meshPropertiesCollection != nullptr ? meshPropertiesCollection->GetVersion() : m_lastMeshPropertiesVersion);
	int materialPropertiesVersion = material->GetProperties().GetVersion();

	if (meshPropertiesVersion != m_lastMeshPropertiesVersion ||
		materialPropertiesVersion != m_lastMaterialPropertiesVersion)
	{
		UpdateBindings(meshPropertiesCollection);

		m_lastMaterialPropertiesVersion = materialPropertiesVersion;
		m_lastMeshPropertiesVersion = meshPropertiesVersion;
	}
}

const Array<std::shared_ptr<IGraphicsResourceSet>>& MaterialRenderData::GetResourceSets()
{
	return m_resourceSets;
}

void MaterialRenderData::Recreate()
{
	const Array<MaterialResourceSet>& resourceSets = m_lastKnownMaterial->GetResourceSets();
	const Array<ShaderBinding>& bindings = m_lastKnownMaterial->GetShader().Get()->GetBindings();
	
	m_uniformBuffers.clear();
	m_resourceSets.clear();

	// Get all resource sets.
	for (const MaterialResourceSet& set : resourceSets)
	{
		if (set.isGlobal)
		{
			m_resourceSets.push_back(m_renderer->GetGlobalResourceSet(set.hashCode));
		}
		else
		{
			m_resourceSets.push_back(m_renderer->AllocateResourceSet(set.description));
		}
	}

	// Create all mesh-specific uniform buffers.
	for (const ShaderBinding& binding : bindings)
	{
		if (binding.Type == GraphicsBindingType::UniformBufferObject)
		{
			if (binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Mesh)
			{
				int dataSize = binding.UniformBufferLayout.GetSize();

				std::shared_ptr<IGraphicsUniformBuffer> uniformBuffer = m_graphics->CreateUniformBuffer(
					StringFormat("%s (%s)", m_lastKnownMaterial->GetName().c_str(), binding.Name.c_str()),
					dataSize);

				m_uniformBuffers.push_back(uniformBuffer);
			}
		}
	}
}

void MaterialRenderData::UpdateBindings(MaterialPropertyCollection* meshPropertiesCollection)
{
	const Array<MaterialResourceSet>& resourceSets = m_lastKnownMaterial->GetResourceSets();

	int index = 0;
	for (auto set : resourceSets)
	{
		if (!set.isGlobal)
		{
			set.UpdateBindings(
				m_renderer,
				m_logger,
				m_uniformBuffers,
				meshPropertiesCollection,
				&m_lastKnownMaterial->GetProperties(),
				m_resourceSets[index]);
		}

		index++;
	}
}