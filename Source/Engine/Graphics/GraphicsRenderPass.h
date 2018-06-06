#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Graphics/GraphicsEnums.h"

#include <memory>

typedef int GraphicsAttachmentIndex;
typedef int GraphicsSubPassIndex;
typedef int GraphicsSubPassDependencyIndex;

#define GraphicsExternalPassIndex static_cast<GraphicsSubPassIndex>(-1)

enum class GraphicsAccessMask
{
	Read = 1,
	Write = 2,
	ReadWrite = 3,
	None = 0
};

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
