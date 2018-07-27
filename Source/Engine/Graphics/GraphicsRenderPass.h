#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Graphics/GraphicsEnums.h"

typedef int GraphicsAttachmentIndex;
typedef int GraphicsSubPassIndex;
typedef int GraphicsSubPassDependencyIndex;

#define GraphicsExternalPassIndex static_cast<GraphicsSubPassIndex>(-1)

struct GraphicsRenderPassAttachment
{
	bool bIsDepthStencil;
	bool bIsPresentBuffer;
	GraphicsFormat format;
};

struct GraphicsRenderPassSubPass
{
};

struct GraphicsRenderPassSubPassDependency
{
	GraphicsSubPassIndex sourcePass;
	GraphicsAccessMask sourceAccessMask;

	GraphicsSubPassIndex destPass;
	GraphicsAccessMask destAccessMask;
};

struct GraphicsRenderPassSettings
{
public:
	Array<GraphicsRenderPassAttachment> attachments;
	Array<GraphicsRenderPassSubPass> subPasses;
	Array<GraphicsRenderPassSubPassDependency> subPassDependencies;

	bool transitionToPresentFormat;

public:
	GraphicsAttachmentIndex AddColorAttachment(GraphicsFormat format, bool isPresentBuffer);
	GraphicsAttachmentIndex AddDepthAttachment(GraphicsFormat format);
	GraphicsSubPassIndex AddSubPass();
	GraphicsSubPassDependencyIndex AddSubPassDependency(GraphicsSubPassIndex sourcePass, GraphicsAccessMask sourceAccessMask, GraphicsSubPassIndex destPass, GraphicsAccessMask destAccessMask);

};

class IGraphicsRenderPass
{
protected:
	IGraphicsRenderPass() { };

public:
	virtual ~IGraphicsRenderPass() { };

};
