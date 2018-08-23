#pragma once
#include "Pch.h"

#include "Engine/Rendering/RenderProperty.h"

#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"

//#include "Engine/Resources/Types/Material.h"
//#include "Engine/Resources/Types/Texture.h"
//#include "Engine/Resources/Types/TextureCube.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"

class IGraphics;
class Logger;
class Texture;
class TextureCube;
class IGraphicsImageView;
class IGraphicsSampler;
class IGraphicsUniformBuffer;
class IGraphicsResourceSet;
struct UniformBufferLayout;
struct MaterialResourceSet;
struct RenderPropertyCollectionRenderData;
class Renderer;

struct RenderPropertyCollection
{
private:
	RenderPropertyCollectionRenderData* m_renderData; // Hidden to prevent cyclic includes.

	Dictionary<RenderPropertyHash, RenderProperty> m_properties;
	int m_version;
	int m_lastUpdateVersion;

	Mutex m_resourceMutex;

protected:
	RenderProperty* GetOrCreate(RenderPropertyHash name);

	void UpdateResourceSets(const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger);
	void UpdateUniformBuffers(const std::shared_ptr<Logger>& logger);

	std::shared_ptr<IGraphicsUniformBuffer> RegisterUniformBuffer(const std::shared_ptr<IGraphics>& graphics, const UniformBufferLayout& layout, const String& name);
	std::shared_ptr<IGraphicsResourceSet> RegisterResourceSet(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<IGraphics>& graphics, const MaterialResourceSet& set);

public:
	RenderPropertyCollection();
	RenderPropertyCollection(const RenderPropertyCollection& other);
	~RenderPropertyCollection();

	int GetVersion();

	void Add(const RenderProperty& prop);

	bool Get(RenderPropertyHash Hash, RenderProperty** binding);

	void Set(RenderPropertyHash name, bool value);
	void Set(RenderPropertyHash name, BVector2 value);
	void Set(RenderPropertyHash name, BVector3 value);
	void Set(RenderPropertyHash name, BVector4 value);

	void Set(RenderPropertyHash name, int32_t value);
	void Set(RenderPropertyHash name, IVector2 value);
	void Set(RenderPropertyHash name, IVector3 value);
	void Set(RenderPropertyHash name, IVector4 value);

	void Set(RenderPropertyHash name, uint32_t value);
	void Set(RenderPropertyHash name, UVector2 value);
	void Set(RenderPropertyHash name, UVector3 value);
	void Set(RenderPropertyHash name, UVector4 value);

	void Set(RenderPropertyHash name, float value);
	void Set(RenderPropertyHash name, Vector2 value);
	void Set(RenderPropertyHash name, Vector3 value);
	void Set(RenderPropertyHash name, Vector4 value);

	void Set(RenderPropertyHash name, double value);
	void Set(RenderPropertyHash name, DVector2 value);
	void Set(RenderPropertyHash name, DVector3 value);
	void Set(RenderPropertyHash name, DVector4 value);

	void Set(RenderPropertyHash name, Matrix2 value);
	void Set(RenderPropertyHash name, Matrix3 value);
	void Set(RenderPropertyHash name, Matrix4 value);

	void Set(RenderPropertyHash name, ResourcePtr<Texture> value);
	void Set(RenderPropertyHash name, ResourcePtr<TextureCube> value);
	void Set(RenderPropertyHash name, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler);

	void UpdateResources(const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger);

	std::shared_ptr<IGraphicsUniformBuffer> GetUniformBuffer(const std::shared_ptr<IGraphics>& graphics, const UniformBufferLayout& layout);
	std::shared_ptr<IGraphicsResourceSet> GetResourceSet(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<IGraphics>& graphics, const MaterialResourceSet& set);

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
