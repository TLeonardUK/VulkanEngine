#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsRenderPass.h"

class IGraphicsImageView;

struct GraphicsFramebufferSettings
{
public:
	std::shared_ptr<IGraphicsRenderPass> renderPass;
	Array<std::shared_ptr<IGraphicsImageView>> attachments;
	int width;
	int height;

public:

};

class IGraphicsFramebuffer
{
protected:
	IGraphicsFramebuffer() { };

public:
	virtual ~IGraphicsFramebuffer() { };

};
