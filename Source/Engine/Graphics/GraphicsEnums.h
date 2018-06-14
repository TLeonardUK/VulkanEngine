#pragma once

#include "Engine/Utilities/Enum.h"

enum_begin_declaration(GraphicsPresentMode)
#include "Engine/Graphics/EGraphicsPresentMode.inc"
enum_end_declaration(GraphicsPresentMode)

enum_begin_declaration(GraphicsPipelineStage)
#include "Engine/Graphics/EGraphicsPipelineStage.inc"
enum_end_declaration(GraphicsPipelineStage)

enum_begin_declaration(GraphicsAccessMask)
#include "Engine/Graphics/EGraphicsAccessMask.inc"
enum_end_declaration(GraphicsAccessMask)

enum_begin_declaration(GraphicsFormat)
#include "Engine/Graphics/EGraphicsFormat.inc"
enum_end_declaration(GraphicsFormat)

enum_begin_declaration(GraphicsBindingFormat)
#include "Engine/Graphics/EGraphicsBindingFormat.inc"
enum_end_declaration(GraphicsBindingFormat)

enum_begin_declaration(GraphicsBindingType)
#include "Engine/Graphics/EGraphicsBindingType.inc"
enum_end_declaration(GraphicsBindingType)

enum_begin_declaration(GraphicsAddressMode)
#include "Engine/Graphics/EGraphicsAddressMode.inc"
enum_end_declaration(GraphicsAddressMode)

enum_begin_declaration(GraphicsFilter)
#include "Engine/Graphics/EGraphicsFilter.inc"
enum_end_declaration(GraphicsFilter)

enum_begin_declaration(GraphicsMipMapMode)
#include "Engine/Graphics/EGraphicsMipMapMode.inc"
enum_end_declaration(GraphicsMipMapMode)

enum_begin_declaration(GraphicsBlendFactor)
#include "Engine/Graphics/EGraphicsBlendFactor.inc"
enum_end_declaration(GraphicsBlendFactor)

enum_begin_declaration(GraphicsBlendOp)
#include "Engine/Graphics/EGraphicsBlendOp.inc"
enum_end_declaration(GraphicsBlendOp)

enum_begin_declaration(GraphicsCullMode)
#include "Engine/Graphics/EGraphicsCullMode.inc"
enum_end_declaration(GraphicsCullMode)

enum_begin_declaration(GraphicsDepthCompareOp)
#include "Engine/Graphics/EGraphicsDepthCompareOp.inc"
enum_end_declaration(GraphicsDepthCompareOp)

enum_begin_declaration(GraphicsFaceWindingOrder)
#include "Engine/Graphics/EGraphicsFaceWindingOrder.inc"
enum_end_declaration(GraphicsFaceWindingOrder)

enum_begin_declaration(GraphicsPolygonMode)
#include "Engine/Graphics/EGraphicsPolygonMode.inc"
enum_end_declaration(GraphicsPolygonMode)

enum_begin_declaration(GraphicsStencilTestCompareOp)
#include "Engine/Graphics/EGraphicsStencilTestCompareOp.inc"
enum_end_declaration(GraphicsStencilTestCompareOp)

enum_begin_declaration(GraphicsStencilTestOp)
#include "Engine/Graphics/EGraphicsStencilTestOp.inc"
enum_end_declaration(GraphicsStencilTestOp)

int GetValueCountForGraphicsBindingFormat(GraphicsBindingFormat format);
int GetByteSizeForGraphicsBindingFormat(GraphicsBindingFormat format);
int GetAlignmentForGraphicsBindingFormat(GraphicsBindingFormat format);