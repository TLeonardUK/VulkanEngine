#pragma once
#include "Pch.h"

#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
//#include "Engine/Resources/Types/Texture.h"
//#include "Engine/Resources/Types/TextureCube.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Types/Math.h"
#include "Engine/Types/String.h"
#include "Engine/Utilities/Json.h"

class Texture;
class TextureCube;
class IGraphicsImageView;
class IGraphicsSampler;

typedef int RenderPropertyHash;
RenderPropertyHash CalculateRenderPropertyHash(const String& name);

struct RenderProperty
{
public:
	GraphicsBindingFormat Format;
	RenderPropertyHash Hash;
	String Name;

	union
	{
		bool		Value_Bool;
		BVector2	Value_Bool2;
		BVector3	Value_Bool3;
		BVector4	Value_Bool4;

		int32_t		Value_Int;
		IVector2	Value_Int2;
		IVector3	Value_Int3;
		IVector4	Value_Int4;

		uint32_t	Value_UInt;
		UVector2	Value_UInt2;
		UVector3	Value_UInt3;
		UVector4	Value_UInt4;

		float		Value_Float;
		Vector2		Value_Float2;
		Vector3		Value_Float3;
		Vector4		Value_Float4;

		double		Value_Double;
		DVector2	Value_Double2;
		DVector3	Value_Double3;
		DVector4	Value_Double4;

		Matrix2		Value_Matrix2;
		Matrix3		Value_Matrix3;
		Matrix4		Value_Matrix4;
	};

	ResourcePtr<Texture>	 Value_Texture;
	ResourcePtr<TextureCube> Value_TextureCube;

	std::shared_ptr<IGraphicsImageView> Value_ImageView;
	std::shared_ptr<IGraphicsSampler> Value_ImageSampler;

	void ParseJsonValue(Array<json>& value);

};