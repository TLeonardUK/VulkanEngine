#include "Pch.h"

#include "Engine/Rendering/RenderPropertyCollection.h"
#include "Engine/Graphics/Graphics.h"

#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/TextureCube.h"

#include "Engine/Rendering/MaterialResourceSet.h"
#include "Engine/Rendering/UniformBufferLayout.h"
#include "Engine/Rendering/Renderer.h"

struct UniformBuffer
{
	std::shared_ptr<IGraphicsUniformBuffer> buffer;
	UniformBufferLayout layout;
};

struct ResourceSet
{
	MaterialResourceSet description;
};

struct RenderPropertyCollectionRenderData
{
	Dictionary<size_t, UniformBuffer> m_uniformBuffers;
	Dictionary<size_t, ResourceSet> m_resourceSets;
};

RenderPropertyCollection::RenderPropertyCollection()
	: m_version(0)
	, m_lastUpdateVersion(-1)
{
	m_renderData = new RenderPropertyCollectionRenderData();
}

RenderPropertyCollection::RenderPropertyCollection(const RenderPropertyCollection& other)
	: m_version(0)
	, m_lastUpdateVersion(-1)
{
	m_properties = other.m_properties;

	m_renderData = new RenderPropertyCollectionRenderData();
}

RenderPropertyCollection::~RenderPropertyCollection()
{
	if (m_renderData != nullptr)
	{
		delete m_renderData;
		m_renderData = nullptr;
	}
}

int RenderPropertyCollection::GetVersion()
{
	return m_version;
}

RenderProperty* RenderPropertyCollection::GetOrCreate(RenderPropertyHash name)
{
	RenderProperty* value;
	if (Get(name, &value))
	{
		return value;
	}

	std::pair<RenderPropertyHash, RenderProperty> pair;
	pair.first = name;
	pair.second.Hash = name;

	m_properties.insert(pair);
	m_version++;

	return &m_properties[name];
}

void RenderPropertyCollection::Add(const RenderProperty& prop)
{
	std::pair<RenderPropertyHash, RenderProperty> pair;
	pair.first = prop.Hash;
	pair.second = prop;

	m_properties.insert(pair);
	m_version++;
}

bool RenderPropertyCollection::Get(RenderPropertyHash name, RenderProperty** result)
{
	auto iter = m_properties.find(name);
	if (iter == m_properties.end())
	{
		return false;
	}

	*result = &iter->second;
	return true;
}

void RenderPropertyCollection::Set(RenderPropertyHash name, bool value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool ||
		prop->Value_Bool != value)
	{
		prop->Format = GraphicsBindingFormat::Bool;
		prop->Value_Bool = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, BVector2 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool2 ||
		prop->Value_Bool2 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool2;
		prop->Value_Bool2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, BVector3 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool3 ||
		prop->Value_Bool3 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool3;
		prop->Value_Bool3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, BVector4 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool4 ||
		prop->Value_Bool4 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool4;
		prop->Value_Bool4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, int32_t value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int ||
		prop->Value_Int != value)
	{
		prop->Format = GraphicsBindingFormat::Int;
		prop->Value_Int = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, IVector2 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int2 ||
		prop->Value_Int2 != value)
	{
		prop->Format = GraphicsBindingFormat::Int2;
		prop->Value_Int2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, IVector3 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int3 ||
		prop->Value_Int3 != value)
	{
		prop->Format = GraphicsBindingFormat::Int3;
		prop->Value_Int3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, IVector4 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int4 ||
		prop->Value_Int4 != value)
	{
		prop->Format = GraphicsBindingFormat::Int4;
		prop->Value_Int4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, uint32_t value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt ||
		prop->Value_UInt != value)
	{
		prop->Format = GraphicsBindingFormat::UInt;
		prop->Value_UInt = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, UVector2 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt2 ||
		prop->Value_UInt2 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt2;
		prop->Value_UInt2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, UVector3 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt3 ||
		prop->Value_UInt3 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt3;
		prop->Value_UInt3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, UVector4 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt4 ||
		prop->Value_UInt4 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt4;
		prop->Value_UInt4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, float value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float ||
		prop->Value_Float != value)
	{
		prop->Format = GraphicsBindingFormat::Float;
		prop->Value_Float = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Vector2 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float2 ||
		prop->Value_Float2 != value)
	{
		prop->Format = GraphicsBindingFormat::Float2;
		prop->Value_Float2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Vector3 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float3 ||
		prop->Value_Float3 != value)
	{
		prop->Format = GraphicsBindingFormat::Float3;
		prop->Value_Float3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Vector4 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float4 ||
		prop->Value_Float4 != value)
	{
		prop->Format = GraphicsBindingFormat::Float4;
		prop->Value_Float4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, double value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double ||
		prop->Value_Double != value)
	{
		prop->Format = GraphicsBindingFormat::Double;
		prop->Value_Double = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, DVector2 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double2 ||
		prop->Value_Double2 != value)
	{
		prop->Format = GraphicsBindingFormat::Double2;
		prop->Value_Double2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, DVector3 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double3 ||
		prop->Value_Double3 != value)
	{
		prop->Format = GraphicsBindingFormat::Double3;
		prop->Value_Double3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, DVector4 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double4 ||
		prop->Value_Double4 != value)
	{
		prop->Format = GraphicsBindingFormat::Double4;
		prop->Value_Double4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Matrix2 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Matrix2 ||
		prop->Value_Matrix2 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix2;
		prop->Value_Matrix2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Matrix3 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Matrix3 ||
		prop->Value_Matrix3 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix3;
		prop->Value_Matrix3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Matrix4 value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Matrix4 ||
		prop->Value_Matrix4 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix4;
		prop->Value_Matrix4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, ResourcePtr<Texture> value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Texture ||
		prop->Value_Texture.Get() != value.Get() ||
		prop->Value_ImageView != nullptr ||
		prop->Value_ImageSampler != nullptr)
	{
		prop->Format = GraphicsBindingFormat::Texture;
		prop->Value_Texture = value;
		prop->Value_ImageView = nullptr;
		prop->Value_ImageSampler = nullptr;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, ResourcePtr<TextureCube> value)
{
	RenderProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::TextureCube ||
		prop->Value_TextureCube.Get() != value.Get())
	{
		prop->Format = GraphicsBindingFormat::TextureCube;
		prop->Value_TextureCube = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler)
{
	RenderProperty* prop = GetOrCreate(name);
	
	if (prop->Format != GraphicsBindingFormat::Texture ||
		prop->Value_Texture.Get() != nullptr ||
		prop->Value_ImageView != imageView ||
		prop->Value_ImageSampler != sampler)
	{
		prop->Format = GraphicsBindingFormat::Texture;
		prop->Value_Texture.Reset();
		prop->Value_ImageView = imageView;
		prop->Value_ImageSampler = sampler;
		m_version++;
	}
}

void RenderPropertyCollection::UpdateResources(const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger)
{
	if (m_version != m_lastUpdateVersion)
	{
		ScopeLock lock(m_resourceMutex);

		if (m_version != m_lastUpdateVersion)
		{
			UpdateUniformBuffers(logger);
			UpdateResourceSets(graphics, logger);

			m_lastUpdateVersion = m_version;
		}
	}
}

void RenderPropertyCollection::UpdateResourceSets(const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger)
{
	Array<std::shared_ptr<IGraphicsUniformBuffer>> emptyUboList;

	for (auto& set : m_renderData->m_resourceSets)
	{
		set.second.description.UpdateBindings(
			logger,
			graphics,
			this,
			set.second.description.set);
	}
}

void RenderPropertyCollection::UpdateUniformBuffers(const std::shared_ptr<Logger>& logger)
{
	RenderPropertyCollection* propCollections[1] = { this };

	for (auto& ubo : m_renderData->m_uniformBuffers)
	{
		ubo.second.layout.FillBuffer(logger, ubo.second.buffer, propCollections, 1);
	}
}

std::shared_ptr<IGraphicsUniformBuffer> RenderPropertyCollection::RegisterUniformBuffer(const std::shared_ptr<IGraphics>& graphics, const UniformBufferLayout& layout, const String& name)
{
	ScopeLock lock(m_resourceMutex);

	auto iter = m_renderData->m_uniformBuffers.find(layout.HashCode);
	if (iter != m_renderData->m_uniformBuffers.end())
	{
		return nullptr;
	}

	int dataSize = layout.GetSize();

	GraphicsResourceSetDescription description;
	description.name = name;
	description.AddBinding(layout.Name, 0, GraphicsBindingType::UniformBufferObject);

	UniformBuffer buffer;
	buffer.layout = layout;
	buffer.buffer = graphics->CreateUniformBuffer(StringFormat("UBO (%s)", layout.Name.c_str()), dataSize);

	m_renderData->m_uniformBuffers.emplace(layout.HashCode, buffer);

	m_version++;

	return buffer.buffer;
}

std::shared_ptr<IGraphicsResourceSet> RenderPropertyCollection::RegisterResourceSet(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<IGraphics>& graphics, const MaterialResourceSet& set)
{
	ScopeLock lock(m_resourceMutex);

	auto iter = m_renderData->m_resourceSets.find(set.hashCode);
	if (iter != m_renderData->m_resourceSets.end())
	{
		return nullptr;
	}

	ResourceSet buffer;
	buffer.description = set;
	buffer.description.set = renderer->AllocateResourceSet(set.description);

	m_renderData->m_resourceSets.emplace(set.hashCode, buffer);

	// Register uniform buffers that the resource-set contains.
	for (const ShaderBinding& binding : set.bindings)
	{
		if (binding.Type == GraphicsBindingType::UniformBufferObject)
		{
			RegisterUniformBuffer(graphics, binding.UniformBufferLayout, StringFormat("%s (%s) UBO", set.name.c_str(), binding.Name.c_str()));
		}
	}

	m_version++;

	return buffer.description.set;
}

std::shared_ptr<IGraphicsUniformBuffer> RenderPropertyCollection::GetUniformBuffer(const std::shared_ptr<IGraphics>& graphics, const UniformBufferLayout& layout)
{
	ScopeLock lock(m_resourceMutex);

	auto iter = m_renderData->m_uniformBuffers.find(layout.HashCode);
	if (iter == m_renderData->m_uniformBuffers.end())
	{
		return RegisterUniformBuffer(graphics, layout, "Unnamed UBO");
	}

	return iter->second.buffer;
}

std::shared_ptr<IGraphicsResourceSet> RenderPropertyCollection::GetResourceSet(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<IGraphics>& graphics, const MaterialResourceSet& set)
{
	ScopeLock lock(m_resourceMutex);

	auto iter = m_renderData->m_resourceSets.find(set.hashCode);
	if (iter == m_renderData->m_resourceSets.end())
	{
		return RegisterResourceSet(renderer, graphics, set);
	}

	return iter->second.description.set;
}
