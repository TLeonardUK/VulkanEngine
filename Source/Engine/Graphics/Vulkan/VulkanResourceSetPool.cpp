#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"
#include "Engine/Types/Hash.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

Statistic Stat_Rendering_Vulkan_ResourceSetPoolCount("Rendering/Vulkan/Resource Set Pool Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

// This whole thing feels very crude. bindings hold image references preventing them getting recycled (change to weak_ptr's).
// if a model uses a different uniform buffer for several frames in a row, any descriptors using other uniform buffers will be recycled, resulting
// in constantly churning descriptor pool (slow hash lookups)!

void VulkanResourceSetBinding::UpdateVulkanObjects(Array<VulkanResourceSetBinding>& objects)
{
	for (auto& obj : objects)
	{
		obj.vkUniformBuffer = obj.uniformBuffer != nullptr ? obj.uniformBuffer->GetGpuBuffer() : nullptr;
		obj.vkSampler = obj.sampler != nullptr ? obj.sampler->GetSampler() : nullptr;
		obj.vkSamplerImageView = obj.samplerImageView != nullptr ? obj.samplerImageView->GetImageView() : nullptr;
	}
}

bool VulkanResourceSetBinding::EqualTo(const VulkanResourceSetBinding& other, bool print) const
{
	if (type != other.type ||
		location != other.location ||
		arrayIndex != other.arrayIndex)
	{
		//if (print) printf("Binding different due to location.\n");
		return false;
	}

	if (type == VulkanResourceSetBindingType::Sampler)
	{
		if (vkSampler != other.sampler->GetSampler())
		{
			//if (print) printf("Binding different due to sampler.\n");
			return false;
		}
		if (vkSamplerImageView != other.samplerImageView->GetImageView())
		{
			//if (print) printf("Binding different due to image view.\n");
			return false;
		}
	}
	else if (type == VulkanResourceSetBindingType::UniformBuffer)
	{
		if (vkUniformBuffer != other.uniformBuffer->GetGpuBuffer())
		{
			//if (print) printf("Binding different due to uniform gpu buffer 0x%p / 0x%p.\n", vkUniformBuffer, other.uniformBuffer->GetGpuBuffer());
			return false;
		}
	}
	else
	{
		assert(false);
		return false;
	}

	return true;
}

bool VulkanResourceSetBinding::BindingsEqualTo(const Array<VulkanResourceSetBinding>& first, const Array<VulkanResourceSetBinding>& second, bool print)
{
	if (first.size() != second.size())
	{
		return false;
	}

	for (int i = 0; i < first.size(); i++)
	{
		if (!first[i].EqualTo(second[i], print))
		{
			return false;
		}
	}

	return true;
}

size_t VulkanResourceSetBinding::GetBindingsHashCode(VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings)
{
	size_t hash = 1;
	CombineHash(hash, reinterpret_cast<void*>(layout));
	CombineHash(hash, bindings.size());

	for (int i = 0; i < bindings.size(); i++)
	{
		const VulkanResourceSetBinding& binding = bindings[i];
		CombineHash(hash, binding.type);
		CombineHash(hash, binding.location);
		CombineHash(hash, binding.arrayIndex);

		if (binding.type == VulkanResourceSetBindingType::Sampler)
		{
			CombineHash(hash, reinterpret_cast<void*>(binding.sampler->GetSampler()));
			CombineHash(hash, reinterpret_cast<void*>(binding.samplerImageView->GetImageView()));
		}
		else if (binding.type == VulkanResourceSetBindingType::UniformBuffer)
		{
			CombineHash(hash, reinterpret_cast<void*>(binding.uniformBuffer->GetGpuBuffer()));
		}
	}

	return hash;
}

void VulkanResourceSetPool::DescriptorList::Add(std::shared_ptr<VulkanResourceSetPool::CachedDescriptor> descriptor)
{
	descriptors.push_back(descriptor);
	descriptorsMap.emplace(descriptor->bindingsHashCode, descriptor);
}

void VulkanResourceSetPool::DescriptorList::Remove(std::shared_ptr<VulkanResourceSetPool::CachedDescriptor> descriptor)
{
	for (auto iter = descriptors.begin(); iter != descriptors.end(); iter++)
	{
		if (*iter == descriptor)
		{
			descriptors.erase(iter);
			break;
		}
	}

	for (auto iter = descriptorsMap.begin(); iter != descriptorsMap.end(); iter++)
	{
		if (iter->second == descriptor)
		{
			iter = descriptorsMap.erase(iter);
			break;
		}
	}
}

std::shared_ptr<VulkanResourceSetPool::CachedDescriptor> VulkanResourceSetPool::DescriptorList::Get(size_t hash, VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings)
{
	auto range = descriptorsMap.equal_range(hash);
	for (auto iter = range.first; iter != range.second; iter++)
	{
		std::shared_ptr<VulkanResourceSetPool::CachedDescriptor>& descriptor = iter->second;

		if (descriptor->layout == layout)
		{
			if (VulkanResourceSetBinding::BindingsEqualTo(descriptor->currentBindings, bindings))
			{
				return descriptor;
			}
		}
	}

	return nullptr;
}

VulkanResourceSetPool::VulkanResourceSetPool(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	std::shared_ptr<VulkanGraphics> graphics
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_graphics(graphics)
	, m_setNameIndex(0)
{
	Stat_Rendering_Vulkan_ResourceSetPoolCount.Add(1);
}

VulkanResourceSetPool::~VulkanResourceSetPool()
{
	Stat_Rendering_Vulkan_ResourceSetPoolCount.Add(-1);
	FreeResources();
}

void VulkanResourceSetPool::FreeResources()
{
	for (auto& descriptor : m_allocatedDescriptors.descriptors)
	{
		m_graphics->QueueDisposal([m_device = m_device, pool = descriptor->pool, set = descriptor->set]() {
			vkFreeDescriptorSets(m_device, pool, 1, &set);
		});
	}

	for (auto& descriptor : m_pendingFreeDescriptors.descriptors)
	{
		m_graphics->QueueDisposal([m_device = m_device, pool = descriptor->pool, set = descriptor->set]() {
			vkFreeDescriptorSets(m_device, pool, 1, &set);
		});
	}

	for (auto& descriptor : m_freeDescriptors.descriptors)
	{
		m_graphics->QueueDisposal([m_device = m_device, pool = descriptor->pool, set = descriptor->set]() {
			vkFreeDescriptorSets(m_device, pool, 1, &set);
		});
	}

	for (auto& layout : m_layouts)
	{
		m_graphics->QueueDisposal([m_device = m_device, layout = layout.layout]() {
			vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
		});
	}

	for (auto& pool : m_pools)
	{
		m_graphics->QueueDisposal([m_device = m_device, pool = pool]() {
			vkDestroyDescriptorPool(m_device, pool, nullptr);
		});
	}

	m_layouts.clear();
	m_pools.clear();
}

String VulkanResourceSetPool::GetName()
{
	return m_name;
}

bool VulkanResourceSetPool::Build()
{
	//m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new resource set pool: %s", m_name.c_str());
	return CreateNewPool();
}

bool VulkanResourceSetPool::CreateNewPool()
{
	Array<VkDescriptorPoolSize> poolSizes(2);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = MaxAllocations;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MaxAllocations;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = MaxAllocations;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkDescriptorPool pool;
	CheckVkResultReturnOnFail(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &pool));

	m_pools.insert(m_pools.begin(), pool);

	return true;
}

bool VulkanResourceSetPool::CreateNewSet(VkDescriptorSetLayout layout, VkDescriptorSet& resultSet, VkDescriptorPool& resultPool)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	for (VkDescriptorPool& pool : m_pools)
	{
		allocInfo.descriptorPool = pool;
		if (vkAllocateDescriptorSets(m_device, &allocInfo, &resultSet) == VK_SUCCESS)
		{
			resultPool = pool;
			return true;
		}
	}

	return false;
}

bool VulkanResourceSetPool::WriteDescriptorSet(VkDescriptorSet set, const Array<VulkanResourceSetBinding>& bindings)
{
	Array<VkDescriptorBufferInfo> bufferInfo(bindings.size());
	Array<VkDescriptorImageInfo> imageInfo(bindings.size());
	Array<VkWriteDescriptorSet> writes(bindings.size());

	int bufferInfoIndex = 0;
	int imageInfoIndex = 0;

	for (int i = 0; i < bindings.size(); i++)
	{
		const VulkanResourceSetBinding& binding = bindings[i];

		VkWriteDescriptorSet& write = writes[i];
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = set;
		write.dstBinding = binding.location;
		write.dstArrayElement = binding.arrayIndex;
		//printf("Writing descriptor: 0x%08x\n", set);

		switch (binding.type)
		{
		case VulkanResourceSetBindingType::Sampler:
			{
				imageInfo[imageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo[imageInfoIndex].imageView = binding.samplerImageView->GetImageView();
				imageInfo[imageInfoIndex].sampler = binding.sampler->GetSampler();

				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pBufferInfo = nullptr;
				write.pImageInfo = &imageInfo[imageInfoIndex];
				write.pTexelBufferView = nullptr;

				imageInfoIndex++;

				break;
			}
		case VulkanResourceSetBindingType::UniformBuffer:
			{
				bufferInfo[bufferInfoIndex].buffer = binding.uniformBuffer->GetGpuBuffer();
				bufferInfo[bufferInfoIndex].offset = 0;
				bufferInfo[bufferInfoIndex].range = binding.uniformBuffer->GetDataSize();

				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				write.descriptorCount = 1;
				write.pBufferInfo = &bufferInfo[bufferInfoIndex];
				write.pImageInfo = nullptr;
				write.pTexelBufferView = nullptr;

				bufferInfoIndex++;
				break;
			}
		}
	}

	vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	return true;
}

VkDescriptorSetLayout VulkanResourceSetPool::RequestLayout(const GraphicsResourceSetDescription& description)
{
	// See if we have a cached layout.
	for (auto& layout : m_layouts)
	{
		if (layout.description.EqualTo(description))
		{
			return layout.layout;
		}
	}

	// Nope, create a new one, generate binding data.
	Array<VkDescriptorSetLayoutBinding> bindings(description.bindings.size());

	for (int i = 0; i < description.bindings.size(); i++)
	{
		const GraphicsResourceSetBinding& bindingDescription = description.bindings[i];
		VkDescriptorSetLayoutBinding& binding = bindings[i];

		binding.binding = bindingDescription.location;
		binding.descriptorCount = bindingDescription.arrayLength;
		binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		binding.pImmutableSamplers = nullptr;

		switch (bindingDescription.type)
		{
		case GraphicsBindingType::UniformBufferObject:
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				break;
			}
		case GraphicsBindingType::Sampler:
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			}
		case GraphicsBindingType::SamplerCube:
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			}
		default:
			{
				assert(false);
			}
		}
	}

	// Create layout.
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	CheckVkResultReturnValueOnFail(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout), nullptr);

	CachedLayout layout;
	layout.description = description;
	layout.layout = descriptorSetLayout;
	m_layouts.push_back(layout);

	return descriptorSetLayout;
}

std::shared_ptr<IGraphicsResourceSet> VulkanResourceSetPool::Allocate(const GraphicsResourceSetDescription& description)
{
	VkDescriptorSetLayout layout = RequestLayout(description);
	if (layout == nullptr)
	{
		return nullptr;
	}

	return std::make_shared<VulkanResourceSet>(
		m_device,
		m_logger,
		StringFormat("%s (%s - %i)", m_name.c_str(), description.name.c_str(), m_setNameIndex++),
		shared_from_this(),
		layout);
}

void VulkanResourceSetPool::FlushPendingFree()
{
	for (size_t i = 0; i < m_pendingFreeDescriptors.descriptors.size(); )
	{
		const std::shared_ptr<CachedDescriptor>& val = m_pendingFreeDescriptors.descriptors[i];

		if (val->lastFrameUsed <= m_graphics->GetSafeRecycleFrameIndex())
		{
			//m_logger->WriteInfo(LogCategory::Vulkan, "Freeing descriptor.");

			m_freeDescriptors.Add(val);
			m_pendingFreeDescriptors.Remove(val);
		}
		else
		{
			i++;
		}
	}
}

std::shared_ptr<VulkanResourceSetPool::CachedDescriptor> VulkanResourceSetPool::AllocDescriptorSet(VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings)
{
	ScopeLock lock(m_descriptorSetMutex);

	std::size_t hashCode = VulkanResourceSetBinding::GetBindingsHashCode(layout, bindings);

	// See if there is a descriptor already in-use that we can use.
	std::shared_ptr<CachedDescriptor> result = m_allocatedDescriptors.Get(hashCode, layout, bindings);
	if (result != nullptr)
	{
		result->allocationCount++;
		return result;
	}

	// See if there is an entry in the free list we can use.
	result = m_freeDescriptors.Get(hashCode, layout, bindings);
	if (result != nullptr)
	{
		m_freeDescriptors.Remove(result);
		m_allocatedDescriptors.Add(result);

		result->allocationCount++;
		return result;
	}

	// Flush pending-free list and see if there is now something available.
	FlushPendingFree();
	result = m_freeDescriptors.Get(hashCode, layout, bindings);
	if (result != nullptr)
	{
		m_freeDescriptors.Remove(result);
		m_allocatedDescriptors.Add(result);

		result->allocationCount++;
		return result;
	}

	// Nothing available, allocate a new descriptor.
	std::shared_ptr<CachedDescriptor> descriptor = std::make_shared<CachedDescriptor>();
	descriptor->bindingsHashCode = hashCode;
	descriptor->currentBindings = bindings;
	descriptor->layout = layout;
	descriptor->allocationCount = 1;

	VulkanResourceSetBinding::UpdateVulkanObjects(descriptor->currentBindings);

	if (!CreateNewSet(layout, descriptor->set, descriptor->pool))
	{
		if (!CreateNewPool())
		{
			m_logger->WriteError(LogCategory::Vulkan, "Failed to allocate new descriptor pool when new descriptor was required.");
			return false;
		}
		if (!CreateNewSet(layout, descriptor->set, descriptor->pool))
		{
			m_logger->WriteError(LogCategory::Vulkan, "Failed to allocate new descriptor after creating new pools.");
			return false;
		}
	}

	WriteDescriptorSet(descriptor->set, descriptor->currentBindings);
	m_allocatedDescriptors.Add(descriptor);

	//m_logger->WriteInfo(LogCategory::Vulkan, "Allocated new descriptor.");

	return descriptor;
}

void VulkanResourceSetPool::FreeDescriptorSet(const std::shared_ptr<CachedDescriptor>& set)
{
	ScopeLock lock(m_descriptorSetMutex);

	//m_logger->WriteInfo(LogCategory::Vulkan, "Added descriptor to pending free.");

	set->allocationCount--;
	if (set->allocationCount == 0)
	{
		set->lastFrameUsed = m_graphics->GetFrameIndex();

		m_allocatedDescriptors.Remove(set);
		m_pendingFreeDescriptors.Add(set);
	}
}