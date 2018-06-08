#include "Engine/Resources/Types/Material.h"

const char* Material::Tag = "Material";

Material::Material(ResourcePtr<Shader> shader, Array<MaterialBinding>& bindings)
	: m_shader(shader)
	, m_bindings(bindings)
{
}


void MaterialBinding::ParseJsonValue(Array<json>& values)
{
	// todo: This seems garbage. Think of a better way to do this.
	switch (Format)
	{
	case GraphicsBindingFormat::Texture:
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