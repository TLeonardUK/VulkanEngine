#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Types/Math.h"

#include "Engine/Utilities/EnumImplementation.h"

enum_begin_implementation(GraphicsPresentMode)
#include "Engine/Graphics/EGraphicsPresentMode.inc"
enum_end_implementation(GraphicsPresentMode)

enum_begin_implementation(GraphicsPipelineStage)
#include "Engine/Graphics/EGraphicsPipelineStage.inc"
enum_end_implementation(GraphicsPipelineStage)

enum_begin_implementation(GraphicsAccessMask)
#include "Engine/Graphics/EGraphicsAccessMask.inc"
enum_end_implementation(GraphicsAccessMask)

enum_begin_implementation(GraphicsFormat)
#include "Engine/Graphics/EGraphicsFormat.inc"
enum_end_implementation(GraphicsFormat)

enum_begin_implementation(GraphicsBindingFormat)
#include "Engine/Graphics/EGraphicsBindingFormat.inc"
enum_end_implementation(GraphicsBindingFormat)

enum_begin_implementation(GraphicsBindingType)
#include "Engine/Graphics/EGraphicsBindingType.inc"
enum_end_implementation(GraphicsBindingType)

enum_begin_implementation(GraphicsAddressMode)
#include "Engine/Graphics/EGraphicsAddressMode.inc"
enum_end_implementation(GraphicsAddressMode)

enum_begin_implementation(GraphicsFilter)
#include "Engine/Graphics/EGraphicsFilter.inc"
enum_end_implementation(GraphicsFilter)

enum_begin_implementation(GraphicsMipMapMode)
#include "Engine/Graphics/EGraphicsMipMapMode.inc"
enum_end_implementation(GraphicsMipMapMode)

enum_begin_implementation(GraphicsBlendFactor)
#include "Engine/Graphics/EGraphicsBlendFactor.inc"
enum_end_implementation(GraphicsBlendFactor)

enum_begin_implementation(GraphicsBlendOp)
#include "Engine/Graphics/EGraphicsBlendOp.inc"
enum_end_implementation(GraphicsBlendOp)

enum_begin_implementation(GraphicsCullMode)
#include "Engine/Graphics/EGraphicsCullMode.inc"
enum_end_implementation(GraphicsCullMode)

enum_begin_implementation(GraphicsDepthCompareOp)
#include "Engine/Graphics/EGraphicsDepthCompareOp.inc"
enum_end_implementation(GraphicsDepthCompareOp)

enum_begin_implementation(GraphicsFaceWindingOrder)
#include "Engine/Graphics/EGraphicsFaceWindingOrder.inc"
enum_end_implementation(GraphicsFaceWindingOrder)

enum_begin_implementation(GraphicsPolygonMode)
#include "Engine/Graphics/EGraphicsPolygonMode.inc"
enum_end_implementation(GraphicsPolygonMode)

enum_begin_implementation(GraphicsStencilTestCompareOp)
#include "Engine/Graphics/EGraphicsStencilTestCompareOp.inc"
enum_end_implementation(GraphicsStencilTestCompareOp)

enum_begin_implementation(GraphicsStencilTestOp)
#include "Engine/Graphics/EGraphicsStencilTestOp.inc"
enum_end_implementation(GraphicsStencilTestOp)

int GetValueCountForGraphicsBindingFormat(GraphicsBindingFormat format)
{
	switch (format)
	{
	case GraphicsBindingFormat::Bool:		return 1;
	case GraphicsBindingFormat::Bool2:		return 2;
	case GraphicsBindingFormat::Bool3:		return 3;
	case GraphicsBindingFormat::Bool4:		return 4;
	case GraphicsBindingFormat::Int:		return 1;
	case GraphicsBindingFormat::Int2:		return 2;
	case GraphicsBindingFormat::Int3:		return 3;
	case GraphicsBindingFormat::Int4:		return 4;
	case GraphicsBindingFormat::UInt:		return 1;
	case GraphicsBindingFormat::UInt2:		return 2;
	case GraphicsBindingFormat::UInt3:		return 3;
	case GraphicsBindingFormat::UInt4:		return 4;
	case GraphicsBindingFormat::Float:		return 1;
	case GraphicsBindingFormat::Float2:		return 2;
	case GraphicsBindingFormat::Float3:		return 3;
	case GraphicsBindingFormat::Float4:		return 4;
	case GraphicsBindingFormat::Double:		return 1;
	case GraphicsBindingFormat::Double2:	return 2;
	case GraphicsBindingFormat::Double3:	return 3;
	case GraphicsBindingFormat::Double4:	return 4;
	case GraphicsBindingFormat::Matrix2:	return 4;
	case GraphicsBindingFormat::Matrix3:	return 9;
	case GraphicsBindingFormat::Matrix4:	return 16;
	case GraphicsBindingFormat::Texture:	return 1;
	}
	return 0;
}

int GetByteSizeForGraphicsBindingFormat(GraphicsBindingFormat format)
{
	switch (format)
	{
	case GraphicsBindingFormat::Bool:		return sizeof(bool);
	case GraphicsBindingFormat::Bool2:		return sizeof(BVector2);
	case GraphicsBindingFormat::Bool3:		return sizeof(BVector3);
	case GraphicsBindingFormat::Bool4:		return sizeof(BVector4);
	case GraphicsBindingFormat::Int:		return sizeof(int32_t);
	case GraphicsBindingFormat::Int2:		return sizeof(IVector2);
	case GraphicsBindingFormat::Int3:		return sizeof(IVector3);
	case GraphicsBindingFormat::Int4:		return sizeof(IVector4);
	case GraphicsBindingFormat::UInt:		return sizeof(uint32_t);
	case GraphicsBindingFormat::UInt2:		return sizeof(UVector2);
	case GraphicsBindingFormat::UInt3:		return sizeof(UVector3);
	case GraphicsBindingFormat::UInt4:		return sizeof(UVector4);
	case GraphicsBindingFormat::Float:		return sizeof(float);
	case GraphicsBindingFormat::Float2:		return sizeof(Vector2);
	case GraphicsBindingFormat::Float3:		return sizeof(Vector3);
	case GraphicsBindingFormat::Float4:		return sizeof(Vector4);
	case GraphicsBindingFormat::Double:		return sizeof(double);
	case GraphicsBindingFormat::Double2:	return sizeof(DVector2);
	case GraphicsBindingFormat::Double3:	return sizeof(DVector3);
	case GraphicsBindingFormat::Double4:	return sizeof(DVector4);
	case GraphicsBindingFormat::Matrix2:	return sizeof(Matrix2);
	case GraphicsBindingFormat::Matrix3:	return sizeof(Matrix3);
	case GraphicsBindingFormat::Matrix4:	return sizeof(Matrix4);
	case GraphicsBindingFormat::Texture:	return 0;
	}
	return 0;
}

int GetAlignmentForGraphicsBindingFormat(GraphicsBindingFormat format)
{
	// Based on 
	// https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#interfaces-resources
	// 
	// A scalar of size N has a base alignment of N.
	// A two - component vector, with components of size N, has a base alignment of 2 N.
	// A three - or four - component vector, with components of size N, has a base alignment of 4 N.
	// An array has a base alignment equal to the base alignment of its element type, rounded up to a multiple of 16.
	// A structure has a base alignment equal to the largest base alignment of any of its members, rounded up to a multiple of 16.
	// A row - major matrix of C columns has a base alignment equal to the base alignment of a vector of C matrix components.
	// A column - major matrix has a base alignment equal to the base alignment of the matrix column type.

	switch (format)
	{
	case GraphicsBindingFormat::Bool:		return sizeof(bool);
	case GraphicsBindingFormat::Bool2:		return sizeof(bool) * 2;
	case GraphicsBindingFormat::Bool3:		return sizeof(bool) * 4;
	case GraphicsBindingFormat::Bool4:		return sizeof(bool) * 4;
	case GraphicsBindingFormat::Int:		return sizeof(int32_t);
	case GraphicsBindingFormat::Int2:		return sizeof(int32_t) * 2;
	case GraphicsBindingFormat::Int3:		return sizeof(int32_t) * 4;
	case GraphicsBindingFormat::Int4:		return sizeof(int32_t) * 4;
	case GraphicsBindingFormat::UInt:		return sizeof(uint32_t);
	case GraphicsBindingFormat::UInt2:		return sizeof(uint32_t) * 2;
	case GraphicsBindingFormat::UInt3:		return sizeof(uint32_t) * 4;
	case GraphicsBindingFormat::UInt4:		return sizeof(uint32_t) * 4;
	case GraphicsBindingFormat::Float:		return sizeof(float);
	case GraphicsBindingFormat::Float2:		return sizeof(float) * 2;
	case GraphicsBindingFormat::Float3:		return sizeof(float) * 4;
	case GraphicsBindingFormat::Float4:		return sizeof(float) * 4;
	case GraphicsBindingFormat::Double:		return sizeof(double);
	case GraphicsBindingFormat::Double2:	return sizeof(double) * 2;
	case GraphicsBindingFormat::Double3:	return sizeof(double) * 4;
	case GraphicsBindingFormat::Double4:	return sizeof(double) * 4;
	case GraphicsBindingFormat::Matrix2:	return sizeof(float) * 2;
	case GraphicsBindingFormat::Matrix3:	return sizeof(float) * 4;
	case GraphicsBindingFormat::Matrix4:	return sizeof(float) * 4;
	case GraphicsBindingFormat::Texture:	return 0;
	}
	return 0;
}