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

int GetValueCountForGraphicsBindingFormat(GraphicsBindingFormat format);
int GetByteSizeForGraphicsBindingFormat(GraphicsBindingFormat format);
