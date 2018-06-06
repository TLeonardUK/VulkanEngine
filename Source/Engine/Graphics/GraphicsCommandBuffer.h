#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Color.h"

#include <memory>

class IGraphicsRenderPass;
class IGraphicsFramebuffer;
class IGraphicsIndexBuffer;
class IGraphicsVertexBuffer;
class IGraphicsPipeline;

class IGraphicsCommandBuffer
{
protected:
	IGraphicsCommandBuffer() { };

public:
	virtual ~IGraphicsCommandBuffer() { };

	virtual void Reset() = 0;

	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual void BeginPass(std::shared_ptr<IGraphicsRenderPass> pass, std::shared_ptr<IGraphicsFramebuffer> framebuffer) = 0;
	virtual void EndPass() = 0;

	virtual void BeginSubPass() = 0;
	virtual void EndSubPass() = 0;

	virtual void SetViewport(int x, int y, int width, int height) = 0;
	virtual void SetScissor(int x, int y, int width, int height) = 0;

	virtual void Clear(std::shared_ptr<IGraphicsImage> image, Color color, float depth, float stencil) = 0;

	virtual void SetPipeline(std::shared_ptr<IGraphicsPipeline> pipeline) = 0;
	virtual void SetVertexBuffer(std::shared_ptr<IGraphicsVertexBuffer> buffer) = 0;
	virtual void SetIndexBuffer(std::shared_ptr<IGraphicsIndexBuffer> buffer) = 0;
	virtual void SetResourceSets(Array<std::shared_ptr<IGraphicsResourceSet>> resourceSets) = 0;

	virtual void DrawElements(int vertexCount, int instanceCount, int vertexOffset, int instanceOffset) = 0;
	virtual void DrawIndexedElements(int indexCount, int instanceCount, int indexOffset, int vertexOffset, int instanceOffset) = 0;

	virtual void Upload(std::shared_ptr<IGraphicsVertexBuffer> buffer) = 0;
	virtual void Upload(std::shared_ptr<IGraphicsIndexBuffer> buffer) = 0;
	virtual void Upload(std::shared_ptr<IGraphicsImage> buffer) = 0;

};
