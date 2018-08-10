#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanGraphics;

class VulkanCommandBuffer 
	: public IGraphicsCommandBuffer
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkCommandBuffer m_commandBuffer;
	VkCommandPool m_commandPool;

	int m_subPassIndex;

	bool m_primary;

	VulkanPipeline* m_activePipeline;
	VulkanRenderPass* m_activeRenderPass;
	VulkanFramebuffer* m_activeFramebuffer;

	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDescriptorSet m_descriptorSetBuffer[VulkanGraphics::MAX_BOUND_DESCRIPTOR_SETS];
	uint32_t m_uniformBufferOffsetBuffer[VulkanGraphics::MAX_BOUND_UBO];

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBufferPool;

	VkCommandBuffer GetCommandBuffer();

	void TransitionImage(VulkanImage* image, VkImageLayout dstLayout);
	void TransitionImage(VkImage image, int mipLevels, VkImageLayout srcLayout, VkImageLayout dstLayout, VkFormat format, int layerCount);

public:
	VulkanCommandBuffer(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		VkCommandBuffer buffer,
		VkCommandPool pool,
		std::shared_ptr<VulkanGraphics> graphics,
		bool bPrimary);

	virtual ~VulkanCommandBuffer();

	virtual void Reset();

	virtual void Begin(const std::shared_ptr<IGraphicsRenderPass>& pass = nullptr, const std::shared_ptr<IGraphicsFramebuffer>& framebuffer = nullptr);
	virtual void End();

	virtual void BeginPass(const std::shared_ptr<IGraphicsRenderPass>& pass, const std::shared_ptr<IGraphicsFramebuffer>& framebuffer, bool bInline = true);
	virtual void EndPass();

	virtual void BeginSubPass();
	virtual void EndSubPass();

	virtual void SetViewport(int x, int y, int width, int height);
	virtual void SetScissor(int x, int y, int width, int height);

	virtual void Clear(const std::shared_ptr<IGraphicsImage>& image, Color color, float depth, float stencil);

	virtual void SetPipeline(const std::shared_ptr<IGraphicsPipeline>& pipeline);
	virtual void SetVertexBuffer(const std::shared_ptr<IGraphicsVertexBuffer>& buffer);
	virtual void SetIndexBuffer(const std::shared_ptr<IGraphicsIndexBuffer>& buffer);

	virtual void TransitionResourceSets(const std::shared_ptr<IGraphicsResourceSet>* values, int count);
	virtual void TransitionResourceSets(const Array<std::shared_ptr<IGraphicsResourceSet>*>& values);
	virtual void SetResourceSets(const std::shared_ptr<IGraphicsResourceSet>* values, int count);

	virtual void DrawElements(int vertexCount, int instanceCount, int vertexOffset, int instanceOffset);
	virtual void DrawIndexedElements(int indexCount, int instanceCount, int indexOffset, int vertexOffset, int instanceOffset);

	virtual void Dispatch(const std::shared_ptr<IGraphicsCommandBuffer>& buffer);

	virtual void Upload(const std::shared_ptr<IGraphicsVertexBuffer>& buffer);
	virtual void Upload(const std::shared_ptr<IGraphicsIndexBuffer>& buffer);
	virtual void Upload(const std::shared_ptr<IGraphicsImage>& buffer);

	void UploadStagingBuffers(Array<VulkanStagingBuffer> buffers);

	virtual void FreeResources();
	virtual String GetName();
};