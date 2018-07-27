#include "Pch.h"

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

enum_begin_implementation(GraphicsBindingFrequency)
#include "Engine/Graphics/EGraphicsBindingFrequency.inc"
enum_end_implementation(GraphicsBindingFrequency)

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

enum_begin_implementation(GraphicsPrimitiveType)
#include "Engine/Graphics/EGraphicsPrimitiveType.inc"
enum_end_implementation(GraphicsPrimitiveType)

enum_begin_implementation(GraphicsStencilTestCompareOp)
#include "Engine/Graphics/EGraphicsStencilTestCompareOp.inc"
enum_end_implementation(GraphicsStencilTestCompareOp)

enum_begin_implementation(GraphicsStencilTestOp)
#include "Engine/Graphics/EGraphicsStencilTestOp.inc"
enum_end_implementation(GraphicsStencilTestOp)

enum_begin_implementation(GraphicsUsage)
#include "Engine/Graphics/EGraphicsUsage.inc"
enum_end_implementation(GraphicsUsage)

int GetValueCountForGraphicsBindingFormat(GraphicsBindingFormat format)
{
	static int lookupTable[static_cast<int>(GraphicsBindingFormat::COUNT)] = {
		1,// Bool
		2,
		3,
		4,
		1, //Int
		2,
		3,
		4,
		1,//UInt
		2,
		3,
		4,
		1,//Float
		2,
		3,
		4,
		1,//Double
		2,
		3,
		4,
		4,//Matrix2
		9,//Matrix3
		16,//Matrix4
		1,//Texture
		1,//TextureCube
	};

	return lookupTable[static_cast<int>(format)];
}

int GetByteSizeForGraphicsBindingFormat(GraphicsBindingFormat format)
{
	static int lookupTable[static_cast<int>(GraphicsBindingFormat::COUNT)] = {
		sizeof(bool),
		sizeof(BVector2),
		sizeof(BVector3),
		sizeof(BVector4),
		sizeof(int32_t),
		sizeof(IVector2),
		sizeof(IVector3),
		sizeof(IVector4),
		sizeof(uint32_t),
		sizeof(UVector2),
		sizeof(UVector3),
		sizeof(UVector4),
		sizeof(float),
		sizeof(Vector2),
		sizeof(Vector3),
		sizeof(Vector4),
		sizeof(double),
		sizeof(DVector2),
		sizeof(DVector3),
		sizeof(DVector4),
		sizeof(Matrix2),
		sizeof(Matrix3),
		sizeof(Matrix4),
		0,
		0,
	};

	return lookupTable[static_cast<int>(format)];
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

	static int lookupTable[static_cast<int>(GraphicsBindingFormat::COUNT)] = {
		sizeof(bool),
		sizeof(bool) * 2,
		sizeof(bool) * 4,
		sizeof(bool) * 4,
		sizeof(int32_t),
		sizeof(int32_t) * 2,
		sizeof(int32_t) * 4,
		sizeof(int32_t) * 4,
		sizeof(uint32_t),
		sizeof(uint32_t) * 2,
		sizeof(uint32_t) * 4,
		sizeof(uint32_t) * 4,
		sizeof(float),
		sizeof(float) * 2,
		sizeof(float) * 4,
		sizeof(float) * 4,
		sizeof(double),
		sizeof(double) * 2,
		sizeof(double) * 4,
		sizeof(double) * 4,
		sizeof(float) * 2,
		sizeof(float) * 4,
		sizeof(float) * 4,
		0,
		0,
	};

	return lookupTable[static_cast<int>(format)];
}