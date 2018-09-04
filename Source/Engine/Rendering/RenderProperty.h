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

struct RenderPropertyImageSamplerValue
{
	std::shared_ptr<IGraphicsImageView> ImageView;
	std::shared_ptr<IGraphicsSampler> ImageSampler;

	RenderPropertyImageSamplerValue() = default;

	RenderPropertyImageSamplerValue(
		std::shared_ptr<IGraphicsImageView> view,
		std::shared_ptr<IGraphicsSampler> sampler
	)
		: ImageView(view)
		, ImageSampler(sampler)
	{
	}
};

struct RenderPropertyValue
{
	union
	{
		bool		Bool;
		BVector2	Bool2;
		BVector3	Bool3;
		BVector4	Bool4;

		int32_t		Int;
		IVector2	Int2;
		IVector3	Int3;
		IVector4	Int4;

		uint32_t	UInt;
		UVector2	UInt2;
		UVector3	UInt3;
		UVector4	UInt4;

		float		Float;
		Vector2		Float2;
		Vector3		Float3;
		Vector4		Float4;

		double		Double;
		DVector2	Double2;
		DVector3	Double3;
		DVector4	Double4;

		Matrix2		Matrix2;
		Matrix3		Matrix3;
		Matrix4		Matrix4;
	};

	ResourcePtr<Texture> Texture;
	ResourcePtr<TextureCube> TextureCube;

	RenderPropertyImageSamplerValue ImageSampler;
};

struct RenderProperty
{
private:

public:
	GraphicsBindingFormat Format;
	RenderPropertyHash Hash;
	String Name;

	Array<RenderPropertyValue> Values;

	RenderProperty();
	void ParseJsonValue(Array<json>& value);

};