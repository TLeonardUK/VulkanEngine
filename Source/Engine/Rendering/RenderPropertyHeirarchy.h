#pragma once
#include "Pch.h"

#include "Engine/Rendering/RenderPropertyCollection.h"
#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Types/Hash.h"

struct RenderPropertyHeirarchyVersion
{
private:
	friend struct RenderPropertyHeirarchy;

	int m_versions[(int)GraphicsBindingFrequency::COUNT];

public:
	int GetFrequencyVersion(GraphicsBindingFrequency frequency) const
	{		
		return m_versions[(int)frequency];
	}

	void Reset()
	{
		for (int i = 0; i < (int)GraphicsBindingFrequency::COUNT; i++)
		{
			m_versions[i] = -1;
		}
	}
};

inline bool operator==(const RenderPropertyHeirarchyVersion& lhs, const RenderPropertyHeirarchyVersion& rhs)
{
	for (int i = 0; i < (int)GraphicsBindingFrequency::COUNT; i++)
	{
		if (lhs.GetFrequencyVersion((GraphicsBindingFrequency)i) != rhs.GetFrequencyVersion((GraphicsBindingFrequency)i))
		{
			return false;
		}
	}
	return true;
}

inline bool operator!=(const RenderPropertyHeirarchyVersion& lhs, const RenderPropertyHeirarchyVersion& rhs)
{
	return !(lhs == rhs);
}

struct RenderPropertyHeirarchy
{
private:
	RenderPropertyCollection* m_collections[(int)GraphicsBindingFrequency::COUNT];

protected:

public:
	void Set(GraphicsBindingFrequency level, RenderPropertyCollection* collection)
	{
		m_collections[(int)level] = collection;
	}

	RenderPropertyCollection* Get(GraphicsBindingFrequency level)
	{
		return m_collections[(int)level];
	}

	RenderPropertyHeirarchyVersion GetVersion()
	{
		RenderPropertyHeirarchyVersion version;
		for (int i = 0; i < (int)GraphicsBindingFrequency::COUNT; i++)
		{
			version.m_versions[i] = (m_collections[i] != nullptr ? m_collections[i]->GetVersion() : -1);
		}
		return version;
	}

	size_t GetHashCode()
	{
		size_t hash = 1;
		for (int i = 0; i < (int)GraphicsBindingFrequency::COUNT; i++)
		{
			CombineHash(hash, m_collections[i]);
		}
		return hash;
	}

};


// RenderPropertyCollection -> RenderPropertyCollection
// RenderProperty -> RenderProperty
//
// RenderPropertyCollection::RegisterUniformBufferLayout()
// RenderPropertyCollection::UpdateUniformBuffers()
//
// RenderPropertyHeirarchy
//	 -> RenderPropertyCollection* collections[HeirarchyLevels::Count]
//
// Shader bindings specify heirarchy level for individual resources. Each UBO in a shader
// binding can only reference resources on its defined heirarchy level.
//
// Heirarchy levels:
//	Global		- GBuffer Samplers, Etc
//	View		- Projection / View Matrices
//	Material	- Material specific texture samplers
//	Mesh		- Model Matrices
//
// Each heirarchy collection is updated by the appropriate code at the appropriate time by calling UpdateUniformBuffers.
// MeshRenderState / direct render call:
//	  -> RenderPropertyHeirarchy::GetResourceSet(hashCode, description)
//	  -> RenderPropertyHeirarchy::GetUniformBuffer(hashCode, layout)
//
// Vast majority of MeshRenderState can be purged with this and a lot simplified
//
// MeshRenderData performs a binding update of the mesh heirarchy level (and is mutex locked) when changed. 
// All other levels are explicitly updated elsewhere.
// MeshRenderData simply should work as an interface for updating the mesh level as well as retrieving
// resourcesets/ubos from the appropriate heirarchy levels.
// Rather than a call to UpdateMeshRenderState, it should be a call to GetRenderData(RenderPropertyHeirarchy) returning resource sets to use as the render data
// instance may be being updated on multiple threads.
// 
