#include "Pch.h"

#include "Engine/Rendering/RenderProperty.h"

RenderPropertyHash CalculateRenderPropertyHash(const String& name)
{
	return static_cast<RenderPropertyHash>(std::hash<std::string>{}(name));
}

RenderProperty::RenderProperty()
	: Format(GraphicsBindingFormat::Undefined)
{
}

void RenderProperty::ParseJsonValue(Array<json>& values)
{
	// todo: support arrays.
	Values.resize(1);

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
		Values[0].Bool = (bool)values[0];
		break;
	}
	case GraphicsBindingFormat::Bool2:
	{
		Values[0].Bool2.x = (bool)values[0];
		Values[0].Bool2.y = (bool)values[1];
		break;
	}
	case GraphicsBindingFormat::Bool3:
	{
		Values[0].Bool3.x = (bool)values[0];
		Values[0].Bool3.y = (bool)values[1];
		Values[0].Bool3.z = (bool)values[2];
		break;
	}
	case GraphicsBindingFormat::Bool4:
	{
		Values[0].Bool4.x = (bool)values[0];
		Values[0].Bool4.y = (bool)values[1];
		Values[0].Bool4.z = (bool)values[2];
		Values[0].Bool4.w = (bool)values[3];
		break;
	}
	case GraphicsBindingFormat::Int:
	{
		Values[0].Int = (int)values[0];
		break;
	}
	case GraphicsBindingFormat::Int2:
	{
		Values[0].Int2.x = (int)values[0];
		Values[0].Int2.y = (int)values[1];
		break;
	}
	case GraphicsBindingFormat::Int3:
	{
		Values[0].Int3.x = (int)values[0];
		Values[0].Int3.y = (int)values[1];
		Values[0].Int3.z = (int)values[2];
		break;
	}
	case GraphicsBindingFormat::Int4:
	{
		Values[0].Int4.x = (int)values[0];
		Values[0].Int4.y = (int)values[1];
		Values[0].Int4.z = (int)values[2];
		Values[0].Int4.w = (int)values[3];
		break;
	}
	case GraphicsBindingFormat::UInt:
	{
		Values[0].UInt = (uint32_t)values[0];
		break;
	}
	case GraphicsBindingFormat::UInt2:
	{
		Values[0].UInt2.x = (uint32_t)values[0];
		Values[0].UInt2.y = (uint32_t)values[1];
		break;
	}
	case GraphicsBindingFormat::UInt3:
	{
		Values[0].UInt3.x = (uint32_t)values[0];
		Values[0].UInt3.y = (uint32_t)values[1];
		Values[0].UInt3.z = (uint32_t)values[2];
		break;
	}
	case GraphicsBindingFormat::UInt4:
	{
		Values[0].UInt4.x = (uint32_t)values[0];
		Values[0].UInt4.y = (uint32_t)values[1];
		Values[0].UInt4.z = (uint32_t)values[2];
		Values[0].UInt4.w = (uint32_t)values[3];
		break;
	}
	case GraphicsBindingFormat::Float:
	{
		Values[0].Float = (float)values[0];
		break;
	}
	case GraphicsBindingFormat::Float2:
	{
		Values[0].Float2.x = (float)values[0];
		Values[0].Float2.y = (float)values[1];
		break;
	}
	case GraphicsBindingFormat::Float3:
	{
		Values[0].Float3.x = (float)values[0];
		Values[0].Float3.y = (float)values[1];
		Values[0].Float3.z = (float)values[2];
		break;
	}
	case GraphicsBindingFormat::Float4:
	{
		Values[0].Float4.x = (float)values[0];
		Values[0].Float4.y = (float)values[1];
		Values[0].Float4.z = (float)values[2];
		Values[0].Float4.w = (float)values[3];
		break;
	}
	case GraphicsBindingFormat::Double:
	{
		Values[0].Double = (double)values[0];
		break;
	}
	case GraphicsBindingFormat::Double2:
	{
		Values[0].Double2.x = (double)values[0];
		Values[0].Double2.y = (double)values[1];
		break;
	}
	case GraphicsBindingFormat::Double3:
	{
		Values[0].Double3.x = (double)values[0];
		Values[0].Double3.y = (double)values[1];
		Values[0].Double3.z = (double)values[2];
		break;
	}
	case GraphicsBindingFormat::Double4:
	{
		Values[0].Double4.x = (double)values[0];
		Values[0].Double4.y = (double)values[1];
		Values[0].Double4.z = (double)values[2];
		Values[0].Double4.w = (double)values[3];
		break;
	}
	case GraphicsBindingFormat::Matrix2:
	{
		Values[0].Matrix2 = Matrix2(
			(float)values[0], (float)values[1],
			(float)values[2], (float)values[3]
		);
		break;
	}
	case GraphicsBindingFormat::Matrix3:
	{
		Values[0].Matrix3 = Matrix3(
			(float)values[0], (float)values[1], (float)values[2],
			(float)values[3], (float)values[4], (float)values[5],
			(float)values[6], (float)values[7], (float)values[8]
		);
		break;
	}
	case GraphicsBindingFormat::Matrix4:
	{
		Values[0].Matrix4 = Matrix4(
			(float)values[0], (float)values[1], (float)values[2], (float)values[3],
			(float)values[4], (float)values[5], (float)values[6], (float)values[7],
			(float)values[8], (float)values[9], (float)values[10], (float)values[11],
			(float)values[12], (float)values[13], (float)values[14], (float)values[15]
		);
		break;
	}
	}
}
