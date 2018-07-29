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
	struct CachedDescriptors
	{
		std::size_t bindingsHashCode;
		int lastFrameUsed;
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

	Array<CachedLayout> m_layouts;
	Array<std::shared_ptr<CachedDescriptors>> m_descriptors;
	MultiDictionary<size_t, std::shared_ptr<CachedDescriptors>> m_descriptorsMap;

	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDevice m_device;
	Array<VkDescriptorPool> m_pools;

	std::mutex m_descriptorSetMutex;

	// todo: Maybe change this to something thats not hard-coded? D:
	const int MaxAllocations = 10000;

private:
	friend class VulkanGraphics;
	friend class VulkanResourceSet;

	bool Build();

	bool RequestDescriptorSetForThisFrame(VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings, VkDescriptorSet& output);
	VkDescriptorSetLayout RequestLayout(const GraphicsResourceSetDescription& description);
	bool WriteDescriptorSet(VkDescriptorSet set, const Array<VulkanResourceSetBinding>& bindings);

	bool AllocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& resultSet, VkDescriptorPool& resultPool);
	bool PruneDescriptors();
	bool CreateNewPool();

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