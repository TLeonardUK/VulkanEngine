#include "Pch.h"

#include "Engine/Resources/Types/MaterialPropertyCollection.h"

MaterialPropertyHash CalculateMaterialPropertyHash(const String& name)
{
	return static_cast<MaterialPropertyHash>(std::hash<std::string>{}(name));
}

MaterialPropertyCollection::MaterialPropertyCollection()
	: m_version(0)
{
}

int MaterialPropertyCollection::GetVersion()
{
	return m_version;
}

MaterialProperty* MaterialPropertyCollection::GetOrCreate(MaterialPropertyHash name)
{
	MaterialProperty* value;
	if (Get(name, &value))
	{
		return value;
	}

	std::pair<MaterialPropertyHash, MaterialProperty> pair;
	pair.first = name;
	pair.second.Hash = name;

	m_properties.insert(pair);
	m_version++;

	return &m_properties[name];
}

void MaterialPropertyCollection::Add(const MaterialProperty& prop)
{
	std::pair<MaterialPropertyHash, MaterialProperty> pair;
	pair.first = prop.Hash;
	pair.second = prop;

	m_properties.insert(pair);
	m_version++;
}

bool MaterialPropertyCollection::Get(MaterialPropertyHash name, MaterialProperty** result)
{
	auto iter = m_properties.find(name);
	if (iter == m_properties.end())
	{
		return false;
	}

	*result = &iter->second;
	return true;
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, bool value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool ||
		prop->Value_Bool != value)
	{
		prop->Format = GraphicsBindingFormat::Bool;
		prop->Value_Bool = value;
		m_version++;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, BVector2 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool2 ||
		prop->Value_Bool2 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool2;
		prop->Value_Bool2 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, BVector3 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool3 ||
		prop->Value_Bool3 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool3;
		prop->Value_Bool3 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, BVector4 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Bool4 ||
		prop->Value_Bool4 != value)
	{
		prop->Format = GraphicsBindingFormat::Bool4;
		prop->Value_Bool4 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, int32_t value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int ||
		prop->Value_Int != value)
	{
		prop->Format = GraphicsBindingFormat::Int;
		prop->Value_Int = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, IVector2 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int2 ||
		prop->Value_Int2 != value)
	{
		prop->Format = GraphicsBindingFormat::Int2;
		prop->Value_Int2 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, IVector3 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int3 ||
		prop->Value_Int3 != value)
	{
		prop->Format = GraphicsBindingFormat::Int3;
		prop->Value_Int3 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, IVector4 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Int4 ||
		prop->Value_Int4 != value)
	{
		prop->Format = GraphicsBindingFormat::Int4;
		prop->Value_Int4 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, uint32_t value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt ||
		prop->Value_UInt != value)
	{
		prop->Format = GraphicsBindingFormat::UInt;
		prop->Value_UInt = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, UVector2 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt2 ||
		prop->Value_UInt2 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt2;
		prop->Value_UInt2 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, UVector3 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt3 ||
		prop->Value_UInt3 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt3;
		prop->Value_UInt3 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, UVector4 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::UInt4 ||
		prop->Value_UInt4 != value)
	{
		prop->Format = GraphicsBindingFormat::UInt4;
		prop->Value_UInt4 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, float value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float ||
		prop->Value_Float != value)
	{
		prop->Format = GraphicsBindingFormat::Float;
		prop->Value_Float = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, Vector2 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float2 ||
		prop->Value_Float2 != value)
	{
		prop->Format = GraphicsBindingFormat::Float2;
		prop->Value_Float2 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, Vector3 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float3 ||
		prop->Value_Float3 != value)
	{
		prop->Format = GraphicsBindingFormat::Float3;
		prop->Value_Float3 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, Vector4 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Float4 ||
		prop->Value_Float4 != value)
	{
		prop->Format = GraphicsBindingFormat::Float4;
		prop->Value_Float4 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, double value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double ||
		prop->Value_Double != value)
	{
		prop->Format = GraphicsBindingFormat::Double;
		prop->Value_Double = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, DVector2 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double2 ||
		prop->Value_Double2 != value)
	{
		prop->Format = GraphicsBindingFormat::Double2;
		prop->Value_Double2 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, DVector3 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double3 ||
		prop->Value_Double3 != value)
	{
		prop->Format = GraphicsBindingFormat::Double3;
		prop->Value_Double3 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, DVector4 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Double4 ||
		prop->Value_Double4 != value)
	{
		prop->Format = GraphicsBindingFormat::Double4;
		prop->Value_Double4 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, Matrix2 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Matrix2 ||
		prop->Value_Matrix2 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix2;
		prop->Value_Matrix2 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, Matrix3 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Matrix3 ||
		prop->Value_Matrix3 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix3;
		prop->Value_Matrix3 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, Matrix4 value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Matrix4 ||
		prop->Value_Matrix4 != value)
	{
		prop->Format = GraphicsBindingFormat::Matrix4;
		prop->Value_Matrix4 = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, ResourcePtr<Texture> value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::Texture ||
		prop->Value_Texture.Get() != value.Get() ||
		prop->Value_ImageView != nullptr ||
		prop->Value_ImageSampler != nullptr)
	{
		prop->Format = GraphicsBindingFormat::Texture;
		prop->Value_Texture = value;
		prop->Value_ImageView = nullptr;
		prop->Value_ImageSampler = nullptr;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, ResourcePtr<TextureCube> value)
{
	MaterialProperty* prop = GetOrCreate(name);

	if (prop->Format != GraphicsBindingFormat::TextureCube ||
		prop->Value_TextureCube.Get() != value.Get())
	{
		prop->Format = GraphicsBindingFormat::TextureCube;
		prop->Value_TextureCube = value;
	}
}

void MaterialPropertyCollection::Set(MaterialPropertyHash name, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler)
{
	MaterialProperty* prop = GetOrCreate(name);
	
	if (prop->Format != GraphicsBindingFormat::Texture ||
		prop->Value_Texture.Get() != nullptr ||
		prop->Value_ImageView != imageView ||
		prop->Value_ImageSampler != sampler)
	{
		prop->Format = GraphicsBindingFormat::Texture;
		prop->Value_Texture.Reset();
		prop->Value_ImageView = imageView;
		prop->Value_ImageSampler = sampler;
	}
}

void MaterialProperty::ParseJsonValue(Array<json>& values)
{
	// todo: This seems garbage. Think of a better way to do this.
	switch (Format)
	{
	case GraphicsBindingFormat::Texture:
	{
		break;
	}
	case GraphicsBindingFormat::TextureCube:
	{
		break;
	}
	case GraphicsBindingFormat::Bool:
	{
		Value_Bool = (bool)values[0];
		break;
	}
	case GraphicsBindingFormat::Bool2:
	{
		Value_Bool2.x = (bool)values[0];
		Value_Bool2.y = (bool)values[1];
		break;
	}
	case GraphicsBindingFormat::Bool3:
	{
		Value_Bool3.x = (bool)values[0];
		Value_Bool3.y = (bool)values[1];
		Value_Bool3.z = (bool)values[2];
		break;
	}
	case GraphicsBindingFormat::Bool4:
	{
		Value_Bool4.x = (bool)values[0];
		Value_Bool4.y = (bool)values[1];
		Value_Bool4.z = (bool)values[2];
		Value_Bool4.w = (bool)values[3];
		break;
	}
	case GraphicsBindingFormat::Int:
	{
		Value_Int = (int)values[0];
		break;
	}
	case GraphicsBindingFormat::Int2:
	{
		Value_Int2.x = (int)values[0];
		Value_Int2.y = (int)values[1];
		break;
	}
	case GraphicsBindingFormat::Int3:
	{
		Value_Int3.x = (int)values[0];
		Value_Int3.y = (int)values[1];
		Value_Int3.z = (int)values[2];
		break;
	}
	case GraphicsBindingFormat::Int4:
	{
		Value_Int4.x = (int)values[0];
		Value_Int4.y = (int)values[1];
		Value_Int4.z = (int)values[2];
		Value_Int4.w = (int)values[3];
		break;
	}
	case GraphicsBindingFormat::UInt:
	{
		Value_UInt = (uint32_t)values[0];
		break;
	}
	case GraphicsBindingFormat::UInt2:
	{
		Value_UInt2.x = (uint32_t)values[0];
		Value_UInt2.y = (uint32_t)values[1];
		break;
	}
	case GraphicsBindingFormat::UInt3:
	{
		Value_UInt3.x = (uint32_t)values[0];
		Value_UInt3.y = (uint32_t)values[1];
		Value_UInt3.z = (uint32_t)values[2];
		break;
	}
	case GraphicsBindingFormat::UInt4:
	{
		Value_UInt4.x = (uint32_t)values[0];
		Value_UInt4.y = (uint32_t)values[1];
		Value_UInt4.z = (uint32_t)values[2];
		Value_UInt4.w = (uint32_t)values[3];
		break;
	}
	case GraphicsBindingFormat::Float:
	{
		Value_Float = (float)values[0];
		break;
	}
	case GraphicsBindingFormat::Float2:
	{
		Value_Float2.x = (float)values[0];
		Value_Float2.y = (float)values[1];
		break;
	}
	case GraphicsBindingFormat::Float3:
	{
		Value_Float3.x = (float)values[0];
		Value_Float3.y = (float)values[1];
		Value_Float3.z = (float)values[2];
		break;
	}
	case GraphicsBindingFormat::Float4:
	{
		Value_Float4.x = (float)values[0];
		Value_Float4.y = (float)values[1];
		Value_Float4.z = (float)values[2];
		Value_Float4.w = (float)values[3];
		break;
	}
	case GraphicsBindingFormat::Double:
	{
		Value_Double = (double)values[0];
		break;
	}
	case GraphicsBindingFormat::Double2:
	{
		Value_Double2.x = (double)values[0];
		Value_Double2.y = (double)values[1];
		break;
	}
	case GraphicsBindingFormat::Double3:
	{
		Value_Double3.x = (double)values[0];
		Value_Double3.y = (double)values[1];
		Value_Double3.z = (double)values[2];
		break;
	}
	case GraphicsBindingFormat::Double4:
	{
		Value_Double4.x = (double)values[0];
		Value_Double4.y = (double)values[1];
		Value_Double4.z = (double)values[2];
		Value_Double4.w = (double)values[3];
		break;
	}
	case GraphicsBindingFormat::Matrix2:
	{
		Value_Matrix2 = Matrix2(
			(float)values[0], (float)values[1],
			(float)values[2], (float)values[3]
		);
		break;
	}
	case GraphicsBindingFormat::Matrix3:
	{
		Value_Matrix3 = Matrix3(
			(float)values[0], (float)values[1], (float)values[2],
			(float)values[3], (float)values[4], (float)values[5],
			(float)values[6], (float)values[7], (float)values[8]
		);
		break;
	}
	case GraphicsBindingFormat::Matrix4:
	{
		Value_Matrix4 = Matrix4(
			(float)values[0], (float)values[1], (float)values[2], (float)values[3],
			(float)values[4], (float)values[5], (float)values[6], (float)values[7],
			(float)values[8], (float)values[9], (float)values[10], (float)values[11],
			(float)values[12], (float)values[13], (float)values[14], (float)values[15]
		);
		break;
	}
	}
}