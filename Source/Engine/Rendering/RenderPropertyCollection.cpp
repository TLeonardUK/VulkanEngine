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

RenderProperty* RenderPropertyCollection::GetOrCreate(RenderPropertyHash name, int arrayLength)
{
	RenderProperty* value;
	if (Get(name, &value))
	{
		if (value->Values.size() != arrayLength)
		{
			value->Values.resize(arrayLength);
			m_version++;
		}
		return value;
	}

	std::pair<RenderPropertyHash, RenderProperty> pair;
	pair.first = name;
	pair.second.Hash = name;
	pair.second.Values.resize(arrayLength);

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
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Bool ||
		prop->Values[0].Bool != value)
	{
		prop->Format = GraphicsBindingFormat::Bool;
		prop->Values[0].Bool = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<bool>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Bool ||
			prop->Values[i].Bool != values[i])
		{
			prop->Format = GraphicsBindingFormat::Bool;
			prop->Values[i].Bool = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, BVector2 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Bool2 ||
		prop->Values[0].Bool2 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool2;
		prop->Values[0].Bool2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<BVector2>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Bool2 ||
			prop->Values[i].Bool2 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Bool2;
			prop->Values[i].Bool2 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, BVector3 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Bool3 ||
		prop->Values[0].Bool3 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool3;
		prop->Values[0].Bool3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<BVector3>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Bool3 ||
			prop->Values[i].Bool3 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Bool3;
			prop->Values[i].Bool3 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, BVector4 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Bool4 ||
		prop->Values[0].Bool4 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool4;
		prop->Values[0].Bool4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<BVector4>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Bool4 ||
			prop->Values[i].Bool4 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Bool4;
			prop->Values[i].Bool4 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, int32_t value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Int ||
		prop->Values[0].Int != value)
	{
		prop->Format = GraphicsBindingFormat::Int;
		prop->Values[0].Int = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<int32_t>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Int ||
			prop->Values[i].Int != values[i])
		{
			prop->Format = GraphicsBindingFormat::Int;
			prop->Values[i].Int = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, IVector2 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Int2 ||
		prop->Values[0].Int2 != value)
	{
		prop->Format = GraphicsBindingFormat::Int2;
		prop->Values[0].Int2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<IVector2>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Int2 ||
			prop->Values[i].Int2 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Int2;
			prop->Values[i].Int2 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, IVector3 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Int3 ||
		prop->Values[0].Int3 != value)
	{
		prop->Format = GraphicsBindingFormat::Int3;
		prop->Values[0].Int3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<IVector3>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Int3 ||
			prop->Values[i].Int3 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Int3;
			prop->Values[i].Int3 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, IVector4 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Int4 ||
		prop->Values[0].Int4 != value)
	{
		prop->Format = GraphicsBindingFormat::Int4;
		prop->Values[0].Int4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<IVector4>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Int4 ||
			prop->Values[i].Int4 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Int4;
			prop->Values[i].Int4 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, uint32_t value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::UInt ||
		prop->Values[0].UInt != value)
	{
		prop->Format = GraphicsBindingFormat::UInt;
		prop->Values[0].UInt = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<uint32_t>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::UInt ||
			prop->Values[i].UInt != values[i])
		{
			prop->Format = GraphicsBindingFormat::UInt;
			prop->Values[i].UInt = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, UVector2 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::UInt2 ||
		prop->Values[0].UInt2 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt2;
		prop->Values[0].UInt2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<UVector2>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::UInt2 ||
			prop->Values[i].UInt2 != values[i])
		{
			prop->Format = GraphicsBindingFormat::UInt2;
			prop->Values[i].UInt2 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, UVector3 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::UInt3 ||
		prop->Values[0].UInt3 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt3;
		prop->Values[0].UInt3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<UVector3>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::UInt3 ||
			prop->Values[i].UInt3 != values[i])
		{
			prop->Format = GraphicsBindingFormat::UInt3;
			prop->Values[i].UInt3 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, UVector4 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::UInt4 ||
		prop->Values[0].UInt4 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt4;
		prop->Values[0].UInt4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<UVector4>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::UInt4 ||
			prop->Values[i].UInt4 != values[i])
		{
			prop->Format = GraphicsBindingFormat::UInt4;
			prop->Values[i].UInt4 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, float value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Float ||
		prop->Values[0].Float != value)
	{
		prop->Format = GraphicsBindingFormat::Float;
		prop->Values[0].Float = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<float>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Float ||
			prop->Values[i].Float != values[i])
		{
			prop->Format = GraphicsBindingFormat::Float;
			prop->Values[i].Float = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Vector2 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Float2 ||
		prop->Values[0].Float2 != value)
	{
		prop->Format = GraphicsBindingFormat::Float2;
		prop->Values[0].Float2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<Vector2>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Float2 ||
			prop->Values[i].Float2 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Float2;
			prop->Values[i].Float2 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Vector3 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Float3 ||
		prop->Values[0].Float3 != value)
	{
		prop->Format = GraphicsBindingFormat::Float3;
		prop->Values[0].Float3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<Vector3>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Float3 ||
			prop->Values[i].Float3 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Float3;
			prop->Values[i].Float3 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Vector4 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Float4 ||
		prop->Values[0].Float4 != value)
	{
		prop->Format = GraphicsBindingFormat::Float4;
		prop->Values[0].Float4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<Vector4>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Float4 ||
			prop->Values[i].Float4 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Float4;
			prop->Values[i].Float4 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, double value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Double ||
		prop->Values[0].Double != value)
	{
		prop->Format = GraphicsBindingFormat::Double;
		prop->Values[0].Double = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<double>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Double ||
			prop->Values[i].Double != values[i])
		{
			prop->Format = GraphicsBindingFormat::Double;
			prop->Values[i].Double = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, DVector2 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Double2 ||
		prop->Values[0].Double2 != value)
	{
		prop->Format = GraphicsBindingFormat::Double2;
		prop->Values[0].Double2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<DVector2>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Double2 ||
			prop->Values[i].Double2 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Double2;
			prop->Values[i].Double2 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, DVector3 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Double3 ||
		prop->Values[0].Double3 != value)
	{
		prop->Format = GraphicsBindingFormat::Double3;
		prop->Values[0].Double3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<DVector3>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Double3 ||
			prop->Values[i].Double3 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Double3;
			prop->Values[i].Double3 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, DVector4 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Double4 ||
		prop->Values[0].Double4 != value)
	{
		prop->Format = GraphicsBindingFormat::Double4;
		prop->Values[0].Double4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<DVector4>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Double4 ||
			prop->Values[i].Double4 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Double4;
			prop->Values[i].Double4 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Matrix2 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Matrix2 ||
		prop->Values[0].Matrix2 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix2;
		prop->Values[0].Matrix2 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<Matrix2>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Matrix2 ||
			prop->Values[i].Matrix2 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Matrix2;
			prop->Values[i].Matrix2 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Matrix3 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Matrix3 ||
		prop->Values[0].Matrix3 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix3;
		prop->Values[0].Matrix3 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<Matrix3>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Matrix3 ||
			prop->Values[i].Matrix3 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Matrix3;
			prop->Values[i].Matrix3 = values[i];
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, Matrix4 value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Matrix4 ||
		prop->Values[0].Matrix4 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix4;
		prop->Values[0].Matrix4 = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<Matrix4>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		if (prop->Format != GraphicsBindingFormat::Matrix4 ||
			prop->Values[i].Matrix4 != values[i])
		{
			prop->Format = GraphicsBindingFormat::Matrix4;
			prop->Values[i].Matrix4 = values[i];
			m_version++;
		}
	}
}


void RenderPropertyCollection::Set(RenderPropertyHash name, ResourcePtr<Texture> value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::Texture ||
		prop->Values[0].Texture.Get() != value.Get() ||
		prop->Values[0].ImageSampler.ImageView != nullptr ||
		prop->Values[0].ImageSampler.ImageSampler != nullptr)
	{
		prop->Format = GraphicsBindingFormat::Texture;
		prop->Values[0].Texture = value;
		prop->Values[0].ImageSampler.ImageView = nullptr;
		prop->Values[0].ImageSampler.ImageSampler = nullptr;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<ResourcePtr<Texture>>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		ResourcePtr<Texture> texturePtr = values[i];

		if (prop->Format != GraphicsBindingFormat::Texture ||
			prop->Values[i].Texture.Get() != texturePtr.Get() ||
			prop->Values[i].ImageSampler.ImageView != nullptr ||
			prop->Values[i].ImageSampler.ImageSampler != nullptr)
		{
			prop->Format = GraphicsBindingFormat::Texture;
			prop->Values[i].Texture = texturePtr;
			prop->Values[i].ImageSampler.ImageView = nullptr;
			prop->Values[i].ImageSampler.ImageSampler = nullptr;
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, ResourcePtr<TextureCube> value)
{
	RenderProperty* prop = GetOrCreate(name, 1);

	if (prop->Format != GraphicsBindingFormat::TextureCube ||
		prop->Values[0].TextureCube.Get() != value.Get())
	{
		prop->Format = GraphicsBindingFormat::TextureCube;
		prop->Values[0].TextureCube = value;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<ResourcePtr<TextureCube>>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		ResourcePtr<TextureCube> texturePtr = values[i];

		if (prop->Format != GraphicsBindingFormat::TextureCube ||
			prop->Values[i].TextureCube.Get() != texturePtr.Get())
		{
			prop->Format = GraphicsBindingFormat::TextureCube;
			prop->Values[i].TextureCube = texturePtr;
			m_version++;
		}
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, RenderPropertyImageSamplerValue sampler)
{
	RenderProperty* prop = GetOrCreate(name, 1);
	
	if (prop->Format != GraphicsBindingFormat::Texture ||
		prop->Values[0].Texture.Get() != nullptr ||
		prop->Values[0].ImageSampler.ImageView != sampler.ImageView ||
		prop->Values[0].ImageSampler.ImageSampler != sampler.ImageSampler)
	{
		prop->Format = GraphicsBindingFormat::Texture;
		prop->Values[0].Texture.Reset();
		prop->Values[0].ImageSampler.ImageView = sampler.ImageView;
		prop->Values[0].ImageSampler.ImageSampler = sampler.ImageSampler;
		m_version++;
	}
}

void RenderPropertyCollection::Set(RenderPropertyHash name, const Array<RenderPropertyImageSamplerValue>& values)
{
	RenderProperty* prop = GetOrCreate(name, (int)values.size());

	for (size_t i = 0; i < values.size(); i++)
	{
		RenderPropertyImageSamplerValue sampler = values[i];

		if (prop->Format != GraphicsBindingFormat::Texture ||
			prop->Values[i].Texture.Get() != nullptr ||
			prop->Values[i].ImageSampler.ImageView != sampler.ImageView ||
			prop->Values[i].ImageSampler.ImageSampler != sampler.ImageSampler)
		{
			prop->Format = GraphicsBindingFormat::Texture;
			prop->Values[i].Texture.Reset();
			prop->Values[i].ImageSampler.ImageView = sampler.ImageView;
			prop->Values[i].ImageSampler.ImageSampler = sampler.ImageSampler;
			m_version++;
		}
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
	for (auto& set : m_renderData->m_resourceSets)
	{
		UpdateResourceSet(set.second, graphics, logger);
	}
}

void RenderPropertyCollection::UpdateResourceSet(ResourceSet& set, const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger)
{
	set.description.UpdateBindings(
		logger,
		graphics,
		this,
		set.description.set);
}

void RenderPropertyCollection::UpdateUniformBuffers(const std::shared_ptr<Logger>& logger)
{
	for (auto& ubo : m_renderData->m_uniformBuffers)
	{
		UpdateUniformBuffer(ubo.second, logger);
	}
}

void RenderPropertyCollection::UpdateUniformBuffer(UniformBuffer& buffer, const std::shared_ptr<Logger>& logger)
{
	RenderPropertyCollection* propCollections[1] = { this };
	buffer.layout.FillBuffer(logger, buffer.buffer, propCollections, 1);
}

std::shared_ptr<IGraphicsUniformBuffer> RenderPropertyCollection::RegisterUniformBuffer(const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger, const UniformBufferLayout& layout, const String& name)
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

	// Fill with current values.
	UpdateUniformBuffer(m_renderData->m_uniformBuffers[layout.HashCode], logger);

	m_version++;
	
	return buffer.buffer;
}

std::shared_ptr<IGraphicsResourceSet> RenderPropertyCollection::RegisterResourceSet(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger, const MaterialResourceSet& set)
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
			RegisterUniformBuffer(graphics, logger, binding.UniformBufferLayout, StringFormat("%s (%s) UBO", set.name.c_str(), binding.Name.c_str()));
		}
	}

	// Fill with current values.
	UpdateResourceSet(m_renderData->m_resourceSets[set.hashCode], graphics, logger);

	m_version++;

	return buffer.description.set;
}

std::shared_ptr<IGraphicsUniformBuffer> RenderPropertyCollection::GetUniformBuffer(const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger, const UniformBufferLayout& layout)
{
	ScopeLock lock(m_resourceMutex);

	auto iter = m_renderData->m_uniformBuffers.find(layout.HashCode);
	if (iter == m_renderData->m_uniformBuffers.end())
	{
		return RegisterUniformBuffer(graphics, logger, layout, "Unnamed UBO");
	}

	return iter->second.buffer;
}

std::shared_ptr<IGraphicsResourceSet> RenderPropertyCollection::GetResourceSet(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<IGraphics>& graphics, const std::shared_ptr<Logger>& logger, const MaterialResourceSet& set)
{
	ScopeLock lock(m_resourceMutex);

	auto iter = m_renderData->m_resourceSets.find(set.hashCode);
	if (iter == m_renderData->m_resourceSets.end())
	{
		return RegisterResourceSet(renderer, graphics, logger, set);
	}

	return iter->second.description.set;
}
