#pragma once
#include "Pch.h"

#include "Engine/Utilities/Enum.h"

// Flags that are written into the gbuffer for individual meshes. Determines
// how they are affected by various rendering passes.
enum class RenderFlags
{
	None			= 0,
	ShadowReciever	= 1,
	ShadowCaster	= 2
};

enum class RenderCommandStage
{
	Global_PreRender,				// Before primary render buffers.
	Global_ShadowMap,				// Shadow map rendering thats not-specific to views (spotlights, pointlights, etc)
	Global_PreViews,				// Before any view rendering begins.

	// Per view stages.
	View_START,
		View_PreRender,				// Before rendering view (namely for resource transitions etc).
		View_ShadowMap,				// Shadow map rendering.
		View_GBuffer,				// GBuffer generation.
		View_ShadowMask,			// Generating shadow mask.
		View_Debug,					// Rendering debug primitives.
		View_Lighting,				// Render lighting information.	
		View_PostRender,			// After gbuffer etc has completed.
		View_PostResolve,			// After gbuffer has been resolved to swapchain.
	View_END,

	Global_PostViews,				// After all views have been rendered.
	Global_PrePresent,				// Just before present, used to transition swap chain to appropriate layout.
};

enum_begin_declaration(FrameBufferTarget)
#include "Engine/Rendering/EFrameBufferTarget.inc"
enum_end_declaration(FrameBufferTarget)