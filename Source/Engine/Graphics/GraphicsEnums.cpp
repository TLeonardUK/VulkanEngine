#include "Engine/Graphics/GraphicsEnums.h"

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