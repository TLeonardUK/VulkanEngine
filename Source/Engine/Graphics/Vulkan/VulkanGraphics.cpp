#include "Pch.h"

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
#include "Engine/Profiling/Profiling.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Types/Set.h"
#include "Engine/Types/Math.h"
#include "Engine/Utilities/Statistic.h"

#include "Engine/Build.h"

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

const Array<const char*> VulkanGraphics::DeviceExtensionsToRequest = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const Array<const char*> VulkanGraphics::DeviceLayersToRequest = {
#if defined(DEBUG_BUILD) || defined(CHECKED_BUILD)
	"VK_LAYER_LUNARG_standard_validation"
#endif
};

const Array<const char*> VulkanGraphics::InstanceExtensionsToRequest = {
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

const Array<const char*> VulkanGraphics::InstanceLayersToRequest = {
#if defined(DEBUG_BUILD) || defined(CHECKED_BUILD)
	"VK_LAYER_LUNARG_standard_validation"
#endif
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
	const int initialDisposalTableSize = 1000;

	m_queuedDisposalTable.resize(initialDisposalTableSize);
	m_queuedDisposalFreeIndices.resize(initialDisposalTableSize);
	m_queuedDisposalAllocatedIndices.reserve(initialDisposalTableSize);

	for (int i = 0; i < initialDisposalTableSize; i++)
	{
		m_queuedDisposalFreeIndices[i] = i;
	}
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

	if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0 ||
		(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0)
	{
		DebugBreak();
	}

	return true;
}

String VulkanGraphics::GetShaderPathPostfix()
{
	return ".spirv";
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

	std::shared_ptr<SdlWindow> sdlWindow = std::static_pointer_cast<SdlWindow>(m_window);
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
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.depthBiasClamp = VK_TRUE;
	deviceFeatures.depthClamp = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;

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
	std::shared_ptr<SdlWindow> sdlWindow = std::static_pointer_cast<SdlWindow>(m_window);
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
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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

		std::shared_ptr<VulkanImage> image = std::make_shared<VulkanImage>(m_logicalDevice, m_logger, "Swap Chain Buffer", swapChainImages[i], m_swapChainFormat.format, extent, false, shared_from_this());
		image->SetVkLayout(VK_IMAGE_LAYOUT_UNDEFINED);// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		m_swapChainImages[i] = image;
		m_swapChainImageViews[i] = std::static_pointer_cast<VulkanImageView>(CreateImageView(StringFormat("Swap Chain Buffer Image View %i", i), image));
	}

	m_logger->WriteSuccess(LogCategory::Vulkan, "Successfully created swapchain with %i images.", swapChainImageCount);

	m_currentFrame = 0;

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
	m_memoryAllocator = std::make_shared<VulkanMemoryAllocator>(m_logicalDevice, m_physicalDeviceInfo.Device, m_logger, shared_from_this(), m_physicalDeviceInfo);
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
	ScopeLock guard(m_resourcesMutex);

	m_logger->WriteInfo(LogCategory::Vulkan, "Destroying vulkan instance.");

	// Wait for device to finish rendering before we tear everything apart.
	if (m_logicalDevice != nullptr)
	{
		vkDeviceWaitIdle(m_logicalDevice);
	}

	PurgeQueuedDisposals();

	DisposeSwapChain();

	for (const std::shared_ptr<IVulkanResource>& resource : m_resources)
	{
		resource->FreeResources();
	}

	PurgeQueuedDisposals();

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
	m_frameFences.clear();
	m_imageAvailableSemaphores.clear();
	m_renderFinishedSemaphores.clear();

	m_memoryAllocator->FreeResources();
	m_memoryAllocator = nullptr;

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
	ProfileScope scope(ProfileColors::Draw, "VulkanGraphics::Present");

	// If swap chain has been regenerated this frame, inform caller.
	if (m_swapChainRegeneratedThisFrame)
	{
		CancelPresent();
		return true;
	}
	
	int swapChainIndex = m_currentFrame % m_swapChainImages.size();

	// Wait for previous frame using this swap buffer image to finish.
	vkWaitForFences(m_logicalDevice, 1, &m_frameFences[swapChainIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_logicalDevice, 1, &m_frameFences[swapChainIndex]);

	// Dispose of any resourses we can.
	CollectGarbage();

	// Acquire next swap chain image.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[swapChainIndex], VK_NULL_HANDLE, &imageIndex);
	assert(imageIndex == swapChainIndex);

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

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[swapChainIndex] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[swapChainIndex] };

	Array<VkCommandBuffer> buffers;
	for (std::shared_ptr<VulkanCommandBuffer> buffer : m_pendingCommandBuffers)
	{
		buffers.push_back(buffer->GetCommandBuffer());
	}

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = (uint32_t)buffers.size();
	submitInfo.pCommandBuffers = buffers.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	CheckVkResultReturnOnFail(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameFences[swapChainIndex]));

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

	m_pendingCommandBuffers.clear();
	m_currentFrame++;
	return false;
}

std::shared_ptr<IGraphicsShader> VulkanGraphics::CreateShader(const String& name, const String& entryPoint, GraphicsPipelineStage stage, const Array<char>& data)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanShader> shader = std::make_shared<VulkanShader>(shared_from_this(), m_logicalDevice, m_logger, name, entryPoint, stage);
	if (!shader->LoadFromArray(data))
	{
		return nullptr;
	}

	m_resources.push_back(shader);
	//m_shaders.push_back(shader);

	return shader;
}

std::shared_ptr<IGraphicsRenderPass> VulkanGraphics::CreateRenderPass(const String& name, const GraphicsRenderPassSettings& settings)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanRenderPass> pass = std::make_shared<VulkanRenderPass>(shared_from_this(), m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_renderPasses.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsPipeline> VulkanGraphics::CreatePipeline(const String& name, const GraphicsPipelineSettings& settings)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanPipeline> pass = std::make_shared<VulkanPipeline>(shared_from_this(), m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_pipelines.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsFramebuffer> VulkanGraphics::CreateFramebuffer(const String& name, const GraphicsFramebufferSettings& settings)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanFramebuffer> pass = std::make_shared<VulkanFramebuffer>(shared_from_this(), m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_framebuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsImageView> VulkanGraphics::CreateImageView(const String& name, std::shared_ptr<IGraphicsImage> image)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanImageView> pass = std::make_shared<VulkanImageView>(shared_from_this(), m_logicalDevice, m_logger, name);
	if (!pass->Build(image))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_imageViews.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsCommandBufferPool> VulkanGraphics::CreateCommandBufferPool(const String& name)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanCommandBufferPool> pass = std::make_shared<VulkanCommandBufferPool>(m_logicalDevice, m_physicalDeviceInfo, m_logger, name, shared_from_this());
	if (!pass->Build())
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_commandBufferPools.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsVertexBuffer> VulkanGraphics::CreateVertexBuffer(const String& name, const VertexBufferBindingDescription& binding, int vertexCount)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanVertexBuffer> pass = std::make_shared<VulkanVertexBuffer>(m_logicalDevice, m_logger, name, m_memoryAllocator, shared_from_this());
	if (!pass->Build(binding, vertexCount))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_vertexBuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsIndexBuffer> VulkanGraphics::CreateIndexBuffer(const String& name, int indexSize, int indexCount)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanIndexBuffer> pass = std::make_shared<VulkanIndexBuffer>(m_logicalDevice, m_logger, name, m_memoryAllocator, shared_from_this());
	if (!pass->Build(indexSize, indexCount))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_indexBuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsUniformBuffer> VulkanGraphics::CreateUniformBuffer(const String& name, int bufferSize)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanUniformBuffer> pass = std::make_shared<VulkanUniformBuffer>(m_logicalDevice, m_logger, name, m_memoryAllocator);
	if (!pass->Build(bufferSize))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_uniformBuffers.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsResourceSetPool> VulkanGraphics::CreateResourceSetPool(const String& name)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanResourceSetPool> pass = std::make_shared<VulkanResourceSetPool>(m_logicalDevice, m_logger, name, shared_from_this());
	if (!pass->Build())
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_resourceSetPools.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsImage> VulkanGraphics::CreateImage(const String& name, int width, int height, int layers, GraphicsFormat format, bool generateMips, GraphicsUsage usage)
{
	ScopeLock guard(m_resourcesMutex);

	std::shared_ptr<VulkanImage> pass = std::make_shared<VulkanImage>(m_logicalDevice, m_logger, name, m_memoryAllocator, shared_from_this());
	if (!pass->Build(width, height, layers, format, generateMips, usage))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_images.push_back(pass);
	return pass;
}

std::shared_ptr<IGraphicsSampler> VulkanGraphics::CreateSampler(const String& name, const SamplerDescription& settings)
{
	ScopeLock guard(m_resourcesMutex);

	// todo: do some caching?

	std::shared_ptr<VulkanSampler> pass = std::make_shared<VulkanSampler>(shared_from_this(), m_logicalDevice, m_logger, name);
	if (!pass->Build(settings))
	{
		return nullptr;
	}

	m_resources.push_back(pass);
	//m_samplers.push_back(pass);
	return pass;
}

void VulkanGraphics::Dispatch(const String& name, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	std::shared_ptr<VulkanCommandBuffer> vkBuffer = std::static_pointer_cast<VulkanCommandBuffer>(buffer);
	//printf("[0x%08x] %s\n", vkBuffer->GetCommandBuffer(), name.c_str());
	m_pendingCommandBuffers.push_back(vkBuffer);
}

void VulkanGraphics::WaitForDeviceIdle()
{
	vkDeviceWaitIdle(m_logicalDevice);
}

GraphicsFormat VulkanGraphics::GetSwapChainFormat()
{
	return VkFormatToGraphicsFormat(m_swapChainFormat.format);
}

int VulkanGraphics::GetFrameIndex()
{
	return m_currentFrame;
}

int VulkanGraphics::GetMaxFramesInFlight()
{
	return static_cast<int>(m_swapChainImages.size());
}

int VulkanGraphics::GetSafeRecycleFrameIndex()
{
	return GetFrameIndex() - (GetMaxFramesInFlight() + 1);
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

bool VulkanGraphics::AllocateStagingBuffer(VulkanAllocation target, int offset, int length, VulkanStagingBuffer& result)
{
	// todo: change to permanently mapped memory.

	VulkanAllocation allocation;

	VulkanStagingBuffer buffer;
	buffer.Length = length;
	buffer.HasDesination = true;
	buffer.Destination = target;
	buffer.DestinationOffset = offset;
	buffer.SourceOffset = 0;

	if (!m_memoryAllocator->CreateBuffer(
		length,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		0,
		buffer.Source))
	{
		return false;
	}

	vmaMapMemory(buffer.Source.Allocator, buffer.Source.Allocation, &buffer.MappedData);

	result = buffer;

	return true;
}

bool VulkanGraphics::AllocateStagingBuffer(int length, VulkanStagingBuffer& result)
{
	// todo: change to permanently mapped memory.

	VulkanAllocation allocation;

	VulkanStagingBuffer buffer;
	buffer.Length = length;
	buffer.HasDesination = false;
	buffer.DestinationOffset = 0;
	buffer.SourceOffset = 0;

	if (!m_memoryAllocator->CreateBuffer(
		length,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		0,
		buffer.Source))
	{
		return false;
	}

	vmaMapMemory(buffer.Source.Allocator, buffer.Source.Allocation, &buffer.MappedData);

	result = buffer;

	return true;
}

void VulkanGraphics::ReleaseStagingBuffer(VulkanStagingBuffer buffer)
{
	vmaUnmapMemory(buffer.Source.Allocator, buffer.Source.Allocation);

	QueueDisposal([=]() mutable {
		m_memoryAllocator->FreeBuffer(buffer.Source);
	});
}

void VulkanGraphics::ValidateDisposal()
{
	// DEBUG DEBUG DEBUG
	{
		Set<int> indices;
		for (size_t i = 0; i < m_queuedDisposalAllocatedIndices.size(); i++)
		{
			int tableIndex = m_queuedDisposalAllocatedIndices[i];
			QueuedDisposal& disposal = m_queuedDisposalTable[tableIndex];

			assert(indices.find(tableIndex) == indices.end());
			indices.emplace(tableIndex);

			assert(disposal.allocated == true);
		}
	}
	{
		Set<int> indices;
		for (size_t i = 0; i < m_queuedDisposalFreeIndices.size(); i++)
		{
			int tableIndex = m_queuedDisposalFreeIndices[i];
			QueuedDisposal& disposal = m_queuedDisposalTable[tableIndex];

			assert(indices.find(tableIndex) == indices.end());
			indices.emplace(tableIndex);

			assert(disposal.allocated == false);
		}
	}
	// DEBUG DEBUG DEBUG

	// DEBUG DEBUG DEBUG
	{
		for (size_t i = 0; i < m_queuedDisposalFreeIndices.size(); i++)
		{
			int tableIndex = m_queuedDisposalFreeIndices[i];
			assert(std::find(m_queuedDisposalAllocatedIndices.begin(), m_queuedDisposalAllocatedIndices.end(), tableIndex) == m_queuedDisposalAllocatedIndices.end());
		}
		for (size_t i = 0; i < m_queuedDisposalAllocatedIndices.size(); i++)
		{
			int tableIndex = m_queuedDisposalAllocatedIndices[i];
			assert(std::find(m_queuedDisposalFreeIndices.begin(), m_queuedDisposalFreeIndices.end(), tableIndex) == m_queuedDisposalFreeIndices.end());
		}
	}
	// DEBUG DEBUG DEBUG
}

void VulkanGraphics::QueueDisposal(QueuedDisposal::DisposalFunction_t function)
{
	ScopeLock lock(m_queuedDisposalMutex);

	if (m_queuedDisposalFreeIndices.size() == 0)
	{
		int index = static_cast<int>(m_queuedDisposalTable.size());
		m_queuedDisposalTable.resize(index + 1);
		m_queuedDisposalFreeIndices.push_back(index);
	}

	//ValidateDisposal();

	int index = m_queuedDisposalFreeIndices[m_queuedDisposalFreeIndices.size() - 1];
	m_queuedDisposalFreeIndices.pop_back();
	m_queuedDisposalAllocatedIndices.push_back(index);

	QueuedDisposal& disposal = m_queuedDisposalTable[index];
	disposal.function = std::move(function);
	disposal.frameIndex = m_currentFrame;
	disposal.allocated = true;

	//ValidateDisposal();
}

void VulkanGraphics::PurgeQueuedDisposals()
{
	ScopeLock lock(m_queuedDisposalMutex);

	//ValidateDisposal();

	for (size_t i = 0; i < m_queuedDisposalAllocatedIndices.size(); i++)
	{
		int tableIndex = m_queuedDisposalAllocatedIndices[i];
		QueuedDisposal& disposal = m_queuedDisposalTable[tableIndex];

		disposal.function();

		assert(disposal.allocated == true);
		disposal.allocated = false;
		disposal.function = nullptr;
	}

	m_queuedDisposalAllocatedIndices.clear();
	m_queuedDisposalFreeIndices.clear();
	for (int i = 0; i < m_queuedDisposalTable.size(); i++)
	{
		m_queuedDisposalFreeIndices.push_back(i);
	}

	//ValidateDisposal();
}

void VulkanGraphics::UpdateQueuedDisposals()
{
	ScopeLock lock(m_queuedDisposalMutex);

	size_t count = m_queuedDisposalAllocatedIndices.size();

	for (int i = 0; i < count; )
	{
		int tableIndex = m_queuedDisposalAllocatedIndices[i];
		QueuedDisposal& disposal = m_queuedDisposalTable[tableIndex];

		if (disposal.frameIndex <= GetSafeRecycleFrameIndex())
		{
			disposal.function();

			assert(disposal.allocated);
			disposal.allocated = false;
			disposal.function = nullptr;

			m_queuedDisposalFreeIndices.push_back(tableIndex);
			m_queuedDisposalAllocatedIndices[i] = m_queuedDisposalAllocatedIndices[count - 1];
			count--;
		}
		else
		{
			i++;
		}
	}

	m_queuedDisposalAllocatedIndices.resize(count);

	//ValidateDisposal();
}

void VulkanGraphics::CollectGarbage()
{
	ProfileScope scope(ProfileColors::Draw, "VulkanGraphics::CollectGarbage");

	ScopeLock guard(m_resourcesMutex);

	if (m_resources.size() > 0)
	{
		m_garbageCollectScanIndex++;
		if (m_garbageCollectScanIndex >= m_resources.size())
		{
			m_garbageCollectScanIndex = 0;
		}

		int scanAmount = Math::Min(MaxGarbageScannedPerFrame, (int)m_resources.size());
		for (int scan = 0; scan < scanAmount; scan++)
		{
			std::shared_ptr<IVulkanResource>& resource = m_resources[m_garbageCollectScanIndex];

			if (resource.use_count() == 1)
			{
				m_logger->WriteInfo(LogCategory::Vulkan, "[%-30s] Unloading, no longer referenced.", resource->GetName().c_str());

				m_resources[m_garbageCollectScanIndex] = m_resources[m_resources.size() - 1];
				m_resources.resize(m_resources.size() - 1);
			}
		}
	}

	UpdateQueuedDisposals();
}

std::shared_ptr<IGraphics> VulkanGraphics::Create(std::shared_ptr<Logger> logger, const String& gameName, int gameVersionMajor, int gameVersionMinor, int gameVersionBuild)
{
	return std::make_shared<VulkanGraphics>(logger, gameName, gameVersionMajor, gameVersionMinor, gameVersionBuild);
}

void VulkanGraphics::UpdateStatistics()
{
	m_memoryAllocator->UpdateStatistics();
}