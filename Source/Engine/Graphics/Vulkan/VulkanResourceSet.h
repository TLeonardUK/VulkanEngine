#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

#include <vulkan/vulkan.h>

class VulkanResourceSet : public IGraphicsResourceSet
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkDescriptorSetLayout m_layout;
	VkDescriptorSet m_set;
	VkDescriptorPool m_pool;

private:
	friend class VulkanGraphics;
	friend class VulkanResourceSetPool;
	friend class VulkanPipeline;
	friend class VulkanCommandBuffer;

	void FreeResources();
	
	VkDescriptorSetLayout GetLayout();
	VkDescriptorSet GetSet();

public:
	VulkanResourceSet(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		VkDescriptorSetLayout layout,
		VkDescriptorSet set,
		VkDescriptorPool pool);

	virtual ~VulkanResourceSet();

	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsUniformBuffer> buffer);
	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView);

};