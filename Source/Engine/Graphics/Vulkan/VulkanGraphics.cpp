#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanExtensions.h"
#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"
#include "Engine/Graphics/Vulkan/VulkanShader.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanPipeline.h"
#include "Engine/Graphics/Vulkan/VulkanFramebuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImage.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBufferPool.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanIndexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"
#include "Engine/Windowing/Sdl/SdlWindow.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Build.h"

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

#include <algorithm>
#include <set>
#include <vector>
#include <cassert>
#include <memory>

const Array<const char*> VulkanGraphics::DeviceExtensionsToRequest = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const Array<const char*> VulkanGraphics::DeviceLayersToRequest = {
	"VK_LAYER_LUNARG_standard_validation"
};

const Array<const char*> VulkanGraphics::InstanceExtensionsToRequest = {
	  VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

const Array<const char*> VulkanGraphics::InstanceLayersToRequest = {
	"VK_LAYER_LUNARG_standard_validation"
}; 

VulkanGraphics::VulkanGraphics(
	std::shared_ptr<Logger> logger, 
	const String& gameName,
	int gameVersionMajor, 
	int gameVersionMinor, 
	int gameVersionBuild)
	: m_logger(logger)
	, m_extensionInfo(logger)
	, m_physicalDeviceInfo(logger)
	, m_gameName(gameName)
	, m_gameVersionMajor(gameVersionMajor)
	, m_gameVersionMinor(gameVersionMinor)
	, m_gameVersionBuild(gameVersionBuild)
	, m_preferedPresentMode(GraphicsPresentMode::VSync)
	, m_currentFrame(0)
	, m_debugCallbackHandle(nullptr)
	, m_instance(nullptr)
	, m_logicalDevice(nullptr)
	, m_windowSurface(nullptr)
	, m_swapChain(nullptr)
	, m_graphicsQueue(nullptr)
	, m_computeQueue(nullptr)
	, m_copyQueue(nullptr)
	, m_presentQueue(nullptr)
	, m_swapChainRegeneratedThisFrame(false)
{
}

VulkanGraphics::~VulkanGraphics()
{
	Dispose();
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanGraphics::DebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg)
{
	/*String message = StringFormat("Validation Message: obj=0x%08x obj_type=0x%08x location=0x%08x code=0x%08x layer=%s: %s",
		obj,
		objType,
		location,
		code,
		layerPrefix,
		msg);
	*/

	String message = StringFormat("%s: %s",
		layerPrefix,
		msg);

	if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0)
	{
		m_logger->WriteError(LogCategory::Vulkan, message);
	}
	else if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 ||
			 (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0)
	{
		m_logger->WriteWarning(LogCategory::Vulkan, message);
	}
	else
	{
		m_logger->WriteInfo(LogCategory::Vulkan, message);
	}

	return true;
}

VkInstance VulkanGraphics::GetInstance() const
{
	return m_instance;
}

const VulkanExtensionInfo& VulkanGraphics::GetExtensionInfo() const
{
	return m_extensionInfo;
}

bool VulkanGraphics::CreateInstance()
{
	uint32_t apiVersionToUse = VK_API_VERSION_1_1;

	m_logger->WriteInfo(LogCategory::Vulkan, "Vulkan api version: %i.%i.%i", VK_VERSION_MAJOR(apiVersionToUse), VK_VERSION_MINOR(apiVersionToUse), VK_VERSION_PATCH(apiVersionToUse));
	m_logger->WriteInfo(LogCategory::Vulkan, "Vulkan header version: %i.%i.%i", VK_VERSION_MAJOR(VK_HEADER_VERSION), VK_VERSION_MINOR(VK_HEADER_VERSION), VK_VERSION_PATCH(VK_HEADER_VERSION));

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_gameName.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(m_gameVersionMajor, m_gameVersionMinor, m_gameVersionBuild);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_BUILD);
	appInfo.apiVersion = apiVersionToUse;

	Array<const char*> extensions = InstanceExtensionsToRequest;
	Array<const char*> layers = InstanceLayersToRequest;

	m_extensionInfo.RemoveUnavailableLayers(layers);

	for (const char* extension : extensions)
	{
		if (!m_extensionInfo.IsExtensionAvailable(extension))
		{
			m_logger->WriteError(LogCategory::Vulkan, "Required instance extension '%s' is not available.", extension);
			return false;
		}
	}

	std::shared_ptr<SdlWindow> sdlWindow = std::dynamic_pointer_cast<SdlWindow>(m_window);
	if (sdlWindow != nullptr)
	{
		unsigned int sdlExtensionCount = 0;
		if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow->GetSdlHandle(), &sdlExtensionCount, nullptr))
		{
			m_logger->WriteError(LogCategory::Vulkan, "SDL_Vulkan_GetInstanceExtensions failed.");
			return false;
		}

		Array<const char*> sdlExtensionNames(sdlExtensionCount);
		if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow->GetSdlHandle(), &sdlExtensionCount, sdlExtensionNames.data()))
		{
			m_logger->WriteError(LogCategory::Vulkan, "SDL_Vulkan_GetInstanceExtensions failed.");
			return false;
		}

		for (const char* name : sdlExtensionNames)
		{
			extensions.push_back(name);
		}
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Using Extensions:");
	for (const char*& name : extensions)
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "\t%s", name);
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Using Validation Layers:");
	for (const char*& name : layers)
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "\t%s", name);
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames = layers.data();

	CheckVkResultReturnOnFail(vkCreateInstance(&createInfo, nullptr, &m_instance));

	m_logger->WriteSuccess(LogCategory::Vulkan, "Created vulkan instance.");

	return true;
}

bool VulkanGraphics::CreateDebugCallback()
{
	if (!m_extensionInfo.IsExtensionAvailable(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "Skipping creating debug callback, extension not supported.");
		return true;
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Creating vulkan debug callback ...");

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = 
		VK_DEBUG_REPORT_ERROR_BIT_EXT | 
		VK_DEBUG_REPORT_WARNING_BIT_EXT | 
		/*VK_DEBUG_REPORT_INFORMATION_BIT_EXT |*/
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | 
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	createInfo.pfnCallback = StaticDebugCallback;
	createInfo.pUserData = this;

	CheckVkResultReturnOnFail(vkCreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, &m_debugCallbackHandle));

	m_logger->WriteSuccess(LogCategory::Vulkan, "Created vulkan debug callback.");

	return true;
}

bool VulkanGraphics::ChoosePhysicalDevice()
{
	uint32_t deviceCount = 0;
	CheckVkResultReturnOnFail(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

	if (deviceCount == 0)
	{
		m_logger->WriteError(LogCategory::Vulkan, "No physical devices found.");
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	CheckVkResultReturnOnFail(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

	m_logger->WriteInfo(LogCategory::Vulkan, "Physical Devices:");

	int bestDeviceScore = -1;

	for (VkPhysicalDevice device : devices)
	{
		VulkanDeviceInfo deviceInfo(m_logger);
		if (!deviceInfo.Populate(device, m_windowSurface))
		{
			return false;
		}

		int deviceScore = 0;

		if (deviceInfo.GetPhysicalDeviceSuitability(&deviceScore))
		{
			if (deviceScore > bestDeviceScore)
			{
				m_physicalDeviceInfo = deviceInfo;
				bestDeviceScore = deviceScore;
			}
		}

		VkDeviceSize localMemory = ((deviceInfo.GetTotalDeviceLocalMemory() / 1024) / 1024);

		m_logger->WriteInfo(LogCategory::Vulkan, 
			"\tname=%s type=%i vendor=%i driverVersion=%i apiVersion=%i localMemory=%iMB score=%i", 
			deviceInfo.Properties.deviceName, 
			deviceInfo.Properties.deviceType,
			deviceInfo.Properties.vendorID,
			deviceInfo.Properties.driverVersion,
			deviceInfo.Properties.apiVersion,
			localMemory,
			deviceScore);
	}

	if (bestDeviceScore < 0)
	{
		m_logger->WriteError(LogCategory::Vulkan, "No suitable physical devices found.");
		return false;
	}

	m_logger->WriteSuccess(LogCategory::Vulkan, "Picked %s as most suitable physical device to use.", m_physicalDeviceInfo.Properties.deviceName);
	m_physicalDeviceInfo.Extensions.Print();

	return true;
}

bool VulkanGraphics::CreateLogicalDevice()
{
	float queuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> graphicsQueueCreateInfos;

	std::set<int> uniqueFamilyIds = {
		m_physicalDeviceInfo.GraphicsQueueFamilyIndex,
		m_physicalDeviceInfo.ComputeQueueFamilyIndex,
		m_physicalDeviceInfo.CopyQueueFamilyIndex,
		m_physicalDeviceInfo.PresentQueueFamilyIndex };

	for (int family : uniqueFamilyIds)
	{
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueFamilyIndex = family;
		info.queueCount = 1;
		info.pQueuePriorities = &queuePriority;

		graphicsQueueCreateInfos.push_back(info);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	Array<const char*> extensions = DeviceExtensionsToRequest;
	Array<const char*> layers = DeviceLayersToRequest;

	m_physicalDeviceInfo.Extensions.RemoveUnavailableLayers(layers);

	for (const char* extension : extensions)
	{
		if (!m_physicalDeviceInfo.Extensions.IsExtensionAvailable(extension))
		{
			m_logger->WriteError(LogCategory::Vulkan, "Required device extension '%s' is not available.", extension);
			return false;
		}
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Using Device Extensions:");
	for (const char*& name : extensions)
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "\t%s", name);
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Using Device Validation Layers:");
	for (const char*& name : layers)
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "\t%s", name);
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = graphicsQueueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)graphicsQueueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	CheckVkResultReturnOnFail(vkCreateDevice(m_physicalDeviceInfo.Device, &createInfo, nullptr, &m_logicalDevice));

	vkGetDeviceQueue(m_logicalDevice, m_physicalDeviceInfo.GraphicsQueueFamilyIndex, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, m_physicalDeviceInfo.ComputeQueueFamilyIndex, 0, &m_computeQueue);
	vkGetDeviceQueue(m_logicalDevice, m_physicalDeviceInfo.CopyQueueFamilyIndex, 0, &m_copyQueue);
	vkGetDeviceQueue(m_logicalDevice, m_physicalDeviceInfo.PresentQueueFamilyIndex, 0, &m_presentQueue);

	m_logger->WriteSuccess(LogCategory::Vulkan, "Created logical device and retrieved queues.");

	return true;
} 

bool VulkanGraphics::CreateSurface()
{
	std::shared_ptr<SdlWindow> sdlWindow = std::dynamic_pointer_cast<SdlWindow>(m_window);
	if (sdlWindow == nullptr)
	{
		m_logger->WriteError(LogCategory::Vulkan, "Window is not an sdl window, no implementation for creating window surface available.");
		return false;
	}

	if (!SDL_Vulkan_CreateSurface(sdlWindow->GetSdlHandle(), m_instance, &m_windowSurface))
	{
		m_logger->WriteError(LogCategory::Vulkan, "SDL_Vulkan_CreateSurface failed.");
		return false;
	}

	m_logger->WriteSuccess(LogCategory::Vulkan, "Created vulkan windowing surface.");

	return true;
}

bool VulkanGraphics::ChooseSwapChainFormat()
{
	// No prefered format, return ideal format.
	if (m_physicalDeviceInfo.SupportedSurfaceFormats.size() == 1 &&
		m_physicalDeviceInfo.SupportedSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		m_swapChainFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		m_logger->WriteInfo(LogCategory::Vulkan, "Device has no prefered swapchain format, using our prefered format.");
		return true;
	}

	// Look to see if prefered format is in supported.
	for (const VkSurfaceFormatKHR& format : m_physicalDeviceInfo.SupportedSurfaceFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			m_swapChainFormat = format;
			m_logger->WriteInfo(LogCategory::Vulkan, "Device has our prefered swapchain format.");
			return true;
		}
	}

	// Our prefered format is not supported :(
	m_logger->WriteError(LogCategory::Vulkan, "Device does not have our prefered swapchain format, failed to setup swapchain.");
	return false;
}

bool VulkanGraphics::ChoosePresentMode()
{
	VkPresentModeKHR preferedMode;
	VkPresentModeKHR nextBestMode = VK_PRESENT_MODE_FIFO_KHR;

	switch (m_preferedPresentMode)
	{
	case GraphicsPresentMode::Immediate:
		{
			preferedMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			break;
		}
	case GraphicsPresentMode::VSync:
		{
			preferedMode = VK_PRESENT_MODE_FIFO_KHR;
			break;
		}
	case GraphicsPresentMode::AdaptiveVSync:
		{
			preferedMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			break;
		}
	case GraphicsPresentMode::TripleBuffer:
		{
			preferedMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	for (const VkPresentModeKHR& mode : m_physicalDeviceInfo.SupportedPresentModes)
	{
		if (mode == preferedMode)
		{
			m_presentMode = mode;
			m_logger->WriteInfo(LogCategory::Vulkan, "Device has prefered presentation mode (%i).", mode);
			return mode;
		}
		else if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			nextBestMode = mode;
		}
	}

	m_presentMode = nextBestMode;
	m_logger->WriteInfo(LogCategory::Vulkan, "Device does not have prefered presentation mode, falling back to next best (%i).", m_presentMode);
	return true;
}

bool VulkanGraphics::ChooseSwapChainExtent()
{
	if (m_physicalDeviceInfo.SurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		m_swapChainExtent = m_physicalDeviceInfo.SurfaceCapabilities.currentExtent;
	}
	else
	{
		uint32_t windowWidth = m_window->GetWidth();
		uint32_t windowHeight = m_window->GetHeight();
		m_swapChainExtent.width = std::max(m_physicalDeviceInfo.SurfaceCapabilities.minImageExtent.width, std::min(m_physicalDeviceInfo.SurfaceCapabilities.maxImageExtent.width, windowWidth));
		m_swapChainExtent.width = std::max(m_physicalDeviceInfo.SurfaceCapabilities.minImageExtent.height, std::min(m_physicalDeviceInfo.SurfaceCapabilities.maxImageExtent.height, windowHeight));
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Swap chain extents chosen as %i x %i.", m_swapChainExtent.width, m_swapChainExtent.height);
	return true;
}

bool VulkanGraphics::CreateSwapChain()
{
	uint32_t swapChainImageCount = 2;
	if (m_presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
	{
		swapChainImageCount = 3;
	}
	if (swapChainImageCount > m_physicalDeviceInfo.SurfaceCapabilities.maxImageCount)
	{
		swapChainImageCount = m_physicalDeviceInfo.SurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_windowSurface;
	createInfo.minImageCount = swapChainImageCount;
	createInfo.imageFormat =  m_swapChainFormat.format;
	createInfo.imageColorSpace = m_swapChainFormat.colorSpace;
	createInfo.imageExtent = m_swapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = m_physicalDeviceInfo.SurfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = m_presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	uint32_t queueFamilyIndices[] = { (uint32_t)m_physicalDeviceInfo.GraphicsQueueFamilyIndex, (uint32_t)m_physicalDeviceInfo.PresentQueueFamilyIndex };

	// If graphics and present use different queues, we have to use concurrent mode for this,
	if (queueFamilyIndices[0] != queueFamilyIndices[1]) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; 
		createInfo.pQueueFamilyIndices = nullptr; 
	}

	CheckVkResultReturnOnFail(vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain));	

	// Get the images that make up the swap chain.
	uint32_t imageCount = 0;
	CheckVkResultReturnOnFail(vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr));

	Array<VkImage> swapChainImages(imageCount);
	CheckVkResultReturnOnFail(vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, swapChainImages.data()));

	// Create image views for each swap chain.
	m_swapChainImages.resize(swapChainImages.size());
	m_swapChainImageViews.resize(swapChainImages.size());
	for (int i = 0; i < swapChainImages.size(); i++)
	{
		VkExtent3D extent { m_swapChainExtent.width, m_swapChainExtent.height, 1 };

		std::shared_ptr<VulkanImage> image = std::make_shared<VulkanImage>(m_logicalDevice, m_logger, "Swap Chain Buffer", swapChainImages[i], m_swapChainFormat.format, extent, false);
		m_swapChainImages[i] = image;
		m_swapChainImageViews[i] = std::dynamic_pointer_cast<VulkanImageView>(CreateImageView(StringFormat("Swap Chain Buffer Image View %i", i), image));
	}

	m_logger->WriteSuccess(LogCategory::Vulkan, "Successfully created swapchain with %i images.", swapChainImageCount);

	return true;
}

bool VulkanGraphics::CreateSyncObjects()
{
	m_imageAvailableSemaphores.resize(m_swapChainImages.size());
	m_renderFinishedSemaphores.resize(m_swapChainImages.size());
	m_frameFences.resize(m_swapChainImages.size());

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < m_swapChainImages.size(); i++)
	{
		CheckVkResultReturnOnFail(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));
		CheckVkResultReturnOnFail(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));
		CheckVkResultReturnOnFail(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_frameFences[i]));
	}

	return true;
}

bool VulkanGraphics::CreateMemoryAllocator()
{
	m_memoryAllocator = std::make_shared<VulkanMemoryAllocator>(m_logicalDevice, m_physicalDeviceInfo.Device, m_logger);
	if (!m_memoryAllocator->Build())
	{
		return false;
	}

	return true;
}

bool VulkanGraphics::RecreateSwapChain()
{
	// Block till device has finished with the swap chain.
	vkDeviceWaitIdle(m_logicalDevice);

	m_swapChainRegeneratedThisFrame = true;
	m_physicalDeviceInfo.Populate(m_physicalDeviceInfo.Device, m_windowSurface);

	DisposeSwapChain();

	return CreateFullSwapchain();
}

bool VulkanGraphics::CreateFullSwapchain()
{
	if (!ChooseSwapChainFormat())
	{
		return false;
	}
	if (!ChoosePresentMode())
	{
		return false;
	}
	if (!ChooseSwapChainExtent())
	{
		return false;
	}
	if (!CreateSwapChain())
	{
		return false;
	}
	
	return true;
}

bool VulkanGraphics::Setup()
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Setting up vulkan library ...");

	if (!m_extensionInfo.Populate())
	{
		return false;
	}
	m_extensionInfo.Print();

	if (!CreateInstance())
	{
		return false;
	}
	if (!LoadVulkanExtensions(*this))
	{
		return false;
	}
	if (!CreateDebugCallback())
	{
		return false;
	}
	if (!CreateSurface())
	{
		return false;
	}
	if (!ChoosePhysicalDevice())
	{
		return false;
	}
	if (!CreateLogicalDevice())
	{
		return false;
	}
	if (!CreateMemoryAllocator())
	{
		return false;
	}
	if (!CreateFullSwapchain())
	{
		return false;
	}
	if (!CreateSyncObjects())
	{
		return false;
	}

	return true;
}

void VulkanGraphics::DisposeSwapChain()
{
	if (m_swapChain != nullptr)
	{
		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
		m_swapChain = nullptr;
	}

	m_swapChainImages.clear();
	m_swapChainImageViews.clear();
}

void VulkanGraphics::Dispose()
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	m_logger->WriteInfo(LogCategory::Vulkan, "Destroying vulkan instance.");

	// Wait for device to finish rendering before we tear everything apart.
	if (m_logicalDevice != nullptr)
	{
		vkDeviceWaitIdle(m_logicalDevice);
	}

	DisposeSwapChain();

	for (const std::shared_ptr<VulkanIndexBuffer>& buffer : m_indexBuffers)
	{
		buffer->FreeResources();
	}
	for (const std::shared_ptr<VulkanVertexBuffer>& buffer : m_vertexBuffers)
	{
		buffer->FreeResources();
	}
	for (const std::shared_ptr<VulkanUniformBuffer>& buffer : m_uniformBuffers)
	{
		buffer->FreeResources();
	}
	for (const std::shared_ptr<VulkanCommandBufferPool>& pass : m_commandBufferPools)
	{
		pass->FreeResources();
	}
	for (const std::shared_ptr<VulkanFramebuffer>& buffer : m_framebuffers)
	{
		buffer->FreeResources();
	}
	for (const std::shared_ptr<VulkanImageView>& view : m_imageViews)
	{
		view->FreeResources();
	}
	for (const std::shared_ptr<VulkanImage>& image : m_images)
	{
		image->FreeResources();
	}
	for (const std::shared_ptr<VulkanSampler>& sampler : m_samplers)
	{
		sampler->FreeResources();
	}
	for (const std::shared_ptr<VulkanPipeline>& pipeline : m_pipelines)
	{
		pipeline->FreeResources();
	}
	for (const std::shared_ptr<VulkanRenderPass>& pass : m_renderPasses)
	{
		pass->FreeResources();
	}
	for (const std::shared_ptr<VulkanResourceSetPool>& pass : m_resourceSetPools)
	{
		pass->FreeResources();
	}	

	m_samplers.clear();
	m_images.clear();
	m_framebuffers.clear();
	m_imageViews.clear();
	m_pipelines.clear();
	m_renderPasses.clear();
	m_commandBufferPools.clear();
	m_vertexBuffers.clear();
	m_indexBuffers.clear();
	m_uniformBuffers.clear();
	m_resourceSetPools.clear();

	for (VkFence& fence : m_frameFences)
	{
		vkDestroyFence(m_logicalDevice, fence, nullptr);
	}
	for (VkSemaphore& semaphore : m_imageAvailableSemaphores)
	{
		vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
	}
	for (VkSemaphore& semaphore : m_renderFinishedSemaphores)
	{
		vkDestroySemaphore(m_logicalDevice, semaphore, nullptr);
	}
	
	for (const std::shared_ptr<VulkanShader>& shader : m_shaders)
	{
		shader->FreeResources();
	}
	m_shaders.clear();

	m_frameFences.clear();
	m_imageAvailableSemaphores.clear();
	m_renderFinishedSemaphores.clear();

	m_memoryAllocator->FreeResources();
	m_memoryAllocator = nullptr;

	if (m_windowSurface != nullptr)
	{
		vkDestroySurfaceKHR(m_instance, m_windowSurface, nullptr);
		m_windowSurface = nullptr;
	}

	if (m_logicalDevice != nullptr)
	{
		vkDestroyDevice(m_logicalDevice, nullptr);
		m_logicalDevice = nullptr;
	}

	if (m_debugCallbackHandle != nullptr)
	{
		vkDestroyDebugReportCallbackEXT(m_instance, m_debugCallbackHandle, nullptr);
		m_debugCallbackHandle = nullptr;
	}

	if (m_instance != nullptr)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = nullptr;
	}
}

void VulkanGraphics::SetPresentMode(GraphicsPresentMode mode)
{
	m_preferedPresentMode = mode;

	if (m_swapChainImages.size() > 0)
	{
		RecreateSwapChain();
	}
}

void VulkanGraphics::CancelPresent()
{
	m_pendingCommandBuffers.clear();
	m_swapChainRegeneratedThisFrame = false;
}

bool VulkanGraphics::Present()
{
	// If swap chain has been regenerated this frame, inform caller.
	if (m_swapChainRegeneratedThisFrame)
	{
		CancelPresent();
		return true;
	}

	// Wait for previous frame using this swap buffer image to finish.
	vkWaitForFences(m_logicalDevice, 1, &m_frameFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_logicalDevice, 1, &m_frameFences[m_currentFrame]);

	// Acquire next swap chain image.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		m_logger->WriteWarning(LogCategory::Vulkan, "Swap chain format out of date, recreating.");
	
		RecreateSwapChain();
		CancelPresent();
		return true;
	}
	else if (result!= VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		m_logger->WriteError(LogCategory::Vulkan, "Failed to acquire swap chain image.");
		CancelPresent();
		return false;
	}

	// Submit command buffer.
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };

	Array<VkCommandBuffer> buffers;
	for (std::shared_ptr<VulkanCommandBuffer> buffer : m_pendingCommandBuffers)
	{
		buffers.push_back(buffer->GetCommandBuffer());
	}

	m_pendingCommandBuffers.clear();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = (uint32_t)buffers.size();
	submitInfo.pCommandBuffers = buffers.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	CheckVkResultReturnOnFail(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameFences[m_currentFrame]));

	// Push image back into swap chain.
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		m_logger->WriteWarning(LogCategory::Vulkan, "Swap chain format out of date or suboptimal, recreating.");

		RecreateSwapChain();
		CancelPresent();
		return true;
	}
	else if (result != VK_SUCCESS)
	{
		m_logger->WriteError(LogCategory::Vulkan, "Failed to present swap chain image!");
		return false;
	}

	m_currentFrame = (m_currentFrame + 1) % m_swapChainImages.size();
	return false;
}

std::shared_ptr<IGraphicsShader> VulkanGraphics::CreateShader(const String& name, const String& entryPoint, GraphicsPipelineStage stage, const Array<char>& data)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanShader> shader = std::make_shared<VulkanShader>(m_logicalDevice, m_logger, name, entryPoint, stage);
	if (!shader->LoadFromArray(data))
	{
		return nullptr;
	}

	m_shaders.push_back(shader);
	return shader;
}

std::shared_ptr<IGraphicsRenderPass> VulkanGraphics::CreateRenderPass(const String& name, const GraphicsRenderPassSettings& settings)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanRenderPass> pass = std::make_shared<VulkanRenderPass>(m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_renderPasses.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsPipeline> VulkanGraphics::CreatePipeline(const String& name, const GraphicsPipelineSettings& settings)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanPipeline> pass = std::make_shared<VulkanPipeline>(m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_pipelines.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsFramebuffer> VulkanGraphics::CreateFramebuffer(const String& name, const GraphicsFramebufferSettings& settings)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanFramebuffer> pass = std::make_shared<VulkanFramebuffer>(m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_framebuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsImageView> VulkanGraphics::CreateImageView(const String& name, std::shared_ptr<IGraphicsImage> image)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanImageView> pass = std::make_shared<VulkanImageView>(m_logicalDevice, m_logger, name);
	if (!pass->Build(image))
	{
		return nullptr;
	}

	m_imageViews.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsCommandBufferPool> VulkanGraphics::CreateCommandBufferPool(const String& name)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanCommandBufferPool> pass = std::make_shared<VulkanCommandBufferPool>(m_logicalDevice, m_physicalDeviceInfo, m_logger, name);
	if (!pass->Build())
	{
		return nullptr;
	}

	m_commandBufferPools.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsVertexBuffer> VulkanGraphics::CreateVertexBuffer(const String& name, const VertexBufferBindingDescription& binding, int vertexCount)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanVertexBuffer> pass = std::make_shared<VulkanVertexBuffer>(m_logicalDevice, m_logger, name, m_memoryAllocator);
	if (!pass->Build(binding, vertexCount))
	{
		return nullptr;
	}

	m_vertexBuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsIndexBuffer> VulkanGraphics::CreateIndexBuffer(const String& name, int indexSize, int indexCount)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanIndexBuffer> pass = std::make_shared<VulkanIndexBuffer>(m_logicalDevice, m_logger, name, m_memoryAllocator);
	if (!pass->Build(indexSize, indexCount))
	{
		return nullptr;
	}

	m_indexBuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsUniformBuffer> VulkanGraphics::CreateUniformBuffer(const String& name, int bufferSize)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanUniformBuffer> pass = std::make_shared<VulkanUniformBuffer>(m_logicalDevice, m_logger, name, m_memoryAllocator);
	if (!pass->Build(bufferSize))
	{
		return nullptr;
	}

	m_uniformBuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsResourceSetPool> VulkanGraphics::CreateResourceSetPool(const String& name)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanResourceSetPool> pass = std::make_shared<VulkanResourceSetPool>(m_logicalDevice, m_logger, name);
	if (!pass->Build())
	{
		return nullptr;
	}

	m_resourceSetPools.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsImage> VulkanGraphics::CreateImage(const String& name, int width, int height, int layers, GraphicsFormat format, bool generateMips)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	std::shared_ptr<VulkanImage> pass = std::make_shared<VulkanImage>(m_logicalDevice, m_logger, name, m_memoryAllocator);
	if (!pass->Build(width, height, layers, format, generateMips))
	{
		return nullptr;
	}

	m_images.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsSampler> VulkanGraphics::CreateSampler(const String& name, const SamplerDescription& settings)
{
	std::lock_guard<std::mutex> guard(m_resourcesMutex);

	// todo: do some caching?

	std::shared_ptr<VulkanSampler> pass = std::make_shared<VulkanSampler>(m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_samplers.push_back(pass);
	return pass;
}

void VulkanGraphics::Dispatch(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	m_pendingCommandBuffers.push_back(std::dynamic_pointer_cast<VulkanCommandBuffer>(buffer));
}

void VulkanGraphics::WaitForDeviceIdle()
{
	vkDeviceWaitIdle(m_logicalDevice);
}

GraphicsFormat VulkanGraphics::GetSwapChainFormat()
{
	return VkFormatToGraphicsFormat(m_swapChainFormat.format);
}

Array<std::shared_ptr<IGraphicsImageView>> VulkanGraphics::GetSwapChainViews()
{
	Array<std::shared_ptr<IGraphicsImageView>> result;
	result.resize(m_swapChainImageViews.size());

	for (int i = 0; i < m_swapChainImageViews.size(); i++)
	{
		result[i] = m_swapChainImageViews[i];
	}

	return result;
}

bool VulkanGraphics::AttachToWindow(std::shared_ptr<IWindow> window)
{
	m_window = window;

	if (!Setup())
	{
		return false;
	}

	return true;
}

std::shared_ptr<IGraphics> VulkanGraphics::Create(std::shared_ptr<Logger> logger, const String& gameName, int gameVersionMajor, int gameVersionMinor, int gameVersionBuild)
{
	return std::make_shared<VulkanGraphics>(logger, gameName, gameVersionMajor, gameVersionMinor, gameVersionBuild);
}