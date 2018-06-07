#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsImage.h"

#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include <vulkan/vulkan.h>

class VulkanImage : public IGraphicsImage
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;
	bool m_isOwner;

	VulkanAllocation m_stagingBuffer;
	VulkanAllocation m_mainImage;

	uint32_t m_mipLevels;

	VkDevice m_device;
	VkImage m_image;
	VkFormat m_format;
	VkExtent3D m_extents;
	bool m_isDepth;

	int m_memorySize;
	
private:
	friend class VulkanGraphics;
	friend class VulkanImageView;
	friend class VulkanCommandBuffer;

	void FreeResources();
	bool Build(int width, int height, GraphicsFormat format, bool generateMips);

	VkBuffer GetStagingBuffer();
	VkImage GetVkImage();
	VkFormat GetVkFormat();
	VkExtent3D GetVkExtents();

public:
	VulkanImage(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name, 
		VkImage image,
		VkFormat format,
		VkExtent3D extents,
		bool isOwner);

	VulkanImage(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanMemoryAllocator> allocator);

	virtual int GetWidth();
	virtual int GetHeight();
	virtual int GetMipLevels();
	virtual GraphicsFormat GetFormat();
	virtual bool IsDepth();

	virtual bool Stage(void* buffer, int offset, int length);

	virtual ~VulkanImage();

};