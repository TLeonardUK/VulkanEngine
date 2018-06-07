#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include <vulkan/vulkan.h>

class VulkanCommandBuffer : public IGraphicsCommandBuffer
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkCommandBuffer m_commandBuffer;
	VkCommandPool m_commandPool;

	int m_subPassIndex;

	std::shared_ptr<VulkanPipeline> m_activePipeline;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBufferPool;

	void FreeResources();

	VkCommandBuffer GetCommandBuffer();

	void TransitionImage(VkImage image, VkFormat format, int mipLevels, VkImageLayout srcLayout, VkImageLayout dstLayout);

public:
	VulkanCommandBuffer(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		VkCommandBuffer buffer,
		VkCommandPool pool);

	virtual ~VulkanCommandBuffer();

	virtual void Reset();

	virtual void Begin();
	virtual void End();

	virtual void BeginPass(std::shared_ptr<IGraphicsRenderPass> pass, std::shared_ptr<IGraphicsFramebuffer> framebuffer);
	virtual void EndPass();

	virtual void BeginSubPass();
	virtual void EndSubPass();

	virtual void SetViewport(int x, int y, int width, int height);
	virtual void SetScissor(int x, int y, int width, int height);

	virtual void Clear(std::shared_ptr<IGraphicsImage> image, Color color, float depth, float stencil);

	virtual void SetPipeline(std::shared_ptr<IGraphicsPipeline> pipeline);
	virtual void SetVertexBuffer(std::shared_ptr<IGraphicsVertexBuffer> buffer);
	virtual void SetIndexBuffer(std::shared_ptr<IGraphicsIndexBuffer> buffer);
	virtual void SetResourceSets(Array<std::shared_ptr<IGraphicsResourceSet>> resourceSets);

	virtual void DrawElements(int vertexCount, int instanceCount, int vertexOffset, int instanceOffset);
	virtual void DrawIndexedElements(int indexCount, int instanceCount, int indexOffset, int vertexOffset, int instanceOffset);

	virtual void Upload(std::shared_ptr<IGraphicsVertexBuffer> buffer);
	virtual void Upload(std::shared_ptr<IGraphicsIndexBuffer> buffer);
	virtual void Upload(std::shared_ptr<IGraphicsImage> buffer);

};