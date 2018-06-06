#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSetPool.h"

#include <vulkan/vulkan.h>

class VulkanResourceSet;

class VulkanResourceSetPool : public IGraphicsResourceSetPool
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkDescriptorPool  m_descriptorPool;

	Array<std::shared_ptr<VulkanResourceSet>> m_allocatedSets;

	// todo: Maybe change this to something thats not hard-coded? D:
	const int MaxAllocations = 4000;

private:
	friend class VulkanGraphics;

	void FreeResources();
	bool Build();

public:
	VulkanResourceSetPool(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanResourceSetPool();

	virtual std::shared_ptr<IGraphicsResourceSet> Allocate(const GraphicsResourceSetDescription& description);

};