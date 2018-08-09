#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Dictionary.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

enum class VulkanResourceSetBindingType
{
	UniformBuffer,
	Sampler,
};

struct VulkanResourceSetBinding
{
	VulkanResourceSetBindingType type;

	int location;
	int arrayIndex;

	std::shared_ptr<VulkanUniformBuffer> uniformBuffer;
	std::shared_ptr<VulkanSampler> sampler;
	std::shared_ptr<VulkanImageView> samplerImageView;

	VkBuffer vkUniformBuffer;
	VkSampler vkSampler;
	VkImageView vkSamplerImageView;

	bool EqualTo(const VulkanResourceSetBinding& other, bool print = false) const;
	static bool BindingsEqualTo(const Array<VulkanResourceSetBinding>& first, const Array<VulkanResourceSetBinding>& second, bool print = false);
	static void UpdateVulkanObjects(Array<VulkanResourceSetBinding>& objects);
	static std::size_t GetBindingsHashCode(VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings);
};

class VulkanResourceSetPool
	: public IGraphicsResourceSetPool
	, public std::enable_shared_from_this<VulkanResourceSetPool>
	, public IVulkanResource
{
private:
	struct CachedDescriptor
	{
		std::size_t bindingsHashCode;
		int lastFrameUsed;
		int allocationCount;
		VkDescriptorSetLayout layout;
		VkDescriptorSet set;
		VkDescriptorPool pool;
		Array<VulkanResourceSetBinding> currentBindings;
	};

	struct CachedLayout
	{
		GraphicsResourceSetDescription description;
		VkDescriptorSetLayout layout;
	};

	struct DescriptorList
	{
		Array<std::shared_ptr<CachedDescriptor>> descriptors;
		MultiDictionary<size_t, std::shared_ptr<CachedDescriptor>> descriptorsMap;

		void Add(std::shared_ptr<CachedDescriptor> descriptor);
		void Remove(std::shared_ptr<CachedDescriptor> descriptor);
		std::shared_ptr<CachedDescriptor> Get(size_t hash, VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings);
	};

	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	DescriptorList m_allocatedDescriptors;
	DescriptorList m_pendingFreeDescriptors;
	DescriptorList m_freeDescriptors;

	VkDevice m_device;
	Array<VkDescriptorPool> m_pools;
	Array<CachedLayout> m_layouts;

	std::mutex m_descriptorSetMutex;

	// todo: Maybe change this to something thats not hard-coded? D:
	const int MaxAllocations = 10000;

private:
	friend class VulkanGraphics;
	friend class VulkanResourceSet;

	bool Build();

	VkDescriptorSetLayout RequestLayout(const GraphicsResourceSetDescription& description);
	bool WriteDescriptorSet(VkDescriptorSet set, const Array<VulkanResourceSetBinding>& bindings);

	bool CreateNewSet(VkDescriptorSetLayout layout, VkDescriptorSet& resultSet, VkDescriptorPool& resultPool);
	bool CreateNewPool();

	std::shared_ptr<CachedDescriptor> AllocDescriptorSet(VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings);
	void FreeDescriptorSet(const std::shared_ptr<CachedDescriptor>& set);

	void FlushPendingFree();

public:
	VulkanResourceSetPool(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanGraphics> graphics);

	virtual ~VulkanResourceSetPool();

	virtual std::shared_ptr<IGraphicsResourceSet> Allocate(const GraphicsResourceSetDescription& description);

	virtual void FreeResources();
	virtual String GetName();

};