#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsImage.h"

#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanImage 
	: public IGraphicsImage
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;
	bool m_isOwner;

	VulkanAllocation m_mainImage;

	uint32_t m_mipLevels;

	VkDevice m_device;
	VkImage m_image;
	VkFormat m_format;
	VkExtent3D m_extents;
	int m_layers;
	bool m_isDepth;

	int m_memorySize;
	int m_layerSize;

	Array<VulkanStagingBuffer> m_stagingBuffers;

	std::shared_ptr<VulkanGraphics> m_graphics;

	std::atomic<VkImageLayout> m_layout;

private:
	friend class VulkanGraphics;
	friend class VulkanImageView;
	friend class VulkanCommandBuffer;

	bool Build(int width, int height, int layers, GraphicsFormat format, bool generateMips, GraphicsUsage usage);

	int GetLayerSize();
	Array<VulkanStagingBuffer> ConsumeStagingBuffers();

	VkImage GetVkImage();
	VkFormat GetVkFormat();
	VkExtent3D GetVkExtents();

	VkImageLayout GetVkLayout();
	void SetVkLayout(VkImageLayout layout);

public:
	VulkanImage(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name, 
		VkImage image,
		VkFormat format,
		VkExtent3D extents,
		bool isOwner,
		std::shared_ptr<VulkanGraphics> graphics);

	VulkanImage(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanMemoryAllocator> allocator,
		std::shared_ptr<VulkanGraphics> graphics);

	virtual int GetWidth();
	virtual int GetHeight();
	virtual int GetMipLevels();
	virtual int GetLayers();
	virtual GraphicsFormat GetFormat();
	virtual bool IsDepth();
	virtual bool IsStencil();

	virtual bool Stage(int layer, void* buffer, int offset, int length);

	virtual ~VulkanImage();

	virtual void FreeResources();
	virtual String GetName();

};