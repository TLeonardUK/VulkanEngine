#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanResourceSet
	: public IGraphicsResourceSet
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkDescriptorSetLayout m_layout;
	std::shared_ptr<VulkanResourceSetPool::CachedDescriptor> m_descriptorSet;

	std::shared_ptr<VulkanResourceSetPool> m_pool;

	Array<VulkanResourceSetBinding> m_currentBindings;

	Mutex m_updateMutex;

private:
	friend class VulkanGraphics;
	friend class VulkanResourceSetPool;
	friend class VulkanPipeline;
	friend class VulkanCommandBuffer;

	VkDescriptorSetLayout GetLayout()
	{
		return m_layout;
	}

	const Array<VulkanResourceSetBinding>& GetBindings() const
	{
		return m_currentBindings;
	}

	void UpdateDescriptorSet();

	void GetUniformBufferOffsets(uint32_t* destination, int* count);
	void GetDescriptorSets(VkDescriptorSet* destination, int* count);

	VulkanResourceSetBinding& GetBinding(int location, int arrayIndex);

	void UpdateResources();

public:
	VulkanResourceSet(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanResourceSetPool> pool,
		VkDescriptorSetLayout layout);

	virtual ~VulkanResourceSet();

	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsUniformBuffer> buffer);
	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView);

	virtual void FreeResources();
	virtual String GetName();

};