#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Color.h"

class IGraphicsRenderPass;
class IGraphicsFramebuffer;
class IGraphicsIndexBuffer;
class IGraphicsVertexBuffer;
class IGraphicsPipeline;
class IGraphicsResourceSet;

class IGraphicsCommandBuffer
{
protected:
	IGraphicsCommandBuffer() { };

public:
	virtual ~IGraphicsCommandBuffer() { };

	virtual void Reset() = 0;

	virtual void Begin(const std::shared_ptr<IGraphicsRenderPass>& pass = nullptr, const std::shared_ptr<IGraphicsFramebuffer>& framebuffer = nullptr) = 0;
	virtual void End() = 0;

	virtual void BeginPass(const std::shared_ptr<IGraphicsRenderPass>& pass, const std::shared_ptr<IGraphicsFramebuffer>& framebuffer, bool bInline = true) = 0;
	virtual void EndPass() = 0;

	virtual void BeginSubPass() = 0;
	virtual void EndSubPass() = 0;

	virtual void SetViewport(int x, int y, int width, int height) = 0;
	virtual void SetScissor(int x, int y, int width, int height) = 0;

	virtual void Clear(const std::shared_ptr<IGraphicsImage>& image, Color color, float depth, float stencil) = 0;

	virtual void TransitionResource(const std::shared_ptr<IGraphicsImage>& image, GraphicsAccessMask mask) = 0;

	virtual void SetPipeline(const std::shared_ptr<IGraphicsPipeline>& pipeline) = 0;
	virtual void SetVertexBuffer(const std::shared_ptr<IGraphicsVertexBuffer>& buffer) = 0;
	virtual void SetIndexBuffer(const std::shared_ptr<IGraphicsIndexBuffer>& buffer) = 0;
	virtual void SetResourceSets(const std::shared_ptr<IGraphicsResourceSet>* values, int count) = 0;

	virtual void DrawElements(int vertexCount, int instanceCount, int vertexOffset, int instanceOffset) = 0;
	virtual void DrawIndexedElements(int indexCount, int instanceCount, int indexOffset, int vertexOffset, int instanceOffset) = 0;

	virtual void Dispatch(const std::shared_ptr<IGraphicsCommandBuffer>& buffer) = 0;

	virtual void Upload(const std::shared_ptr<IGraphicsVertexBuffer>& buffer) = 0;
	virtual void Upload(const std::shared_ptr<IGraphicsIndexBuffer>& buffer) = 0;
	virtual void Upload(const std::shared_ptr<IGraphicsImage>& buffer) = 0;

};
