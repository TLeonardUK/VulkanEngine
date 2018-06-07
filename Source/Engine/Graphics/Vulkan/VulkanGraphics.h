#pragma once

#include "Engine/Types/String.h"

#include <memory>
#include <vector>

#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"
#include "Engine/Graphics/Vulkan/VulkanExtensionInfo.h"
#include "Engine/Graphics/Vulkan/VulkanShader.h"

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

#include <thread>
#include <mutex>

class Logger;
class IWindow;

class VulkanRenderPass;
class VulkanShader;
class VulkanPipeline;
class VulkanFramebuffer;
class VulkanImage;
class VulkanImageView;
class VulkanCommandBufferPool;
class VulkanCommandBuffer;
class VulkanVertexBuffer;
class VulkanIndexBuffer;
class VulkanUniformBuffer;
class VulkanMemoryAllocator;
class VulkanResourceSetPool;
class VulkanSampler;

#define CheckVkResultReturnOnFail(call) \
{ \
	VkResult _result_ = call; \
	if (_result_ != VK_SUCCESS) \
	{ \
		m_logger->WriteError(LogCategory::Vulkan, #call " failed, error was: 0x%08x", (int)_result_); \
		return false; \
	} \
} 

#define CheckVkResultReturnVoidOnFail(call) \
{ \
	VkResult _result_ = call; \
	if (_result_ != VK_SUCCESS) \
	{ \
		m_logger->WriteError(LogCategory::Vulkan, #call " failed, error was: 0x%08x", (int)_result_); \
		return; \
	} \
} 

#define CheckVkResultReturnValueOnFail(call, value) \
{ \
	VkResult _result_ = call; \
	if (_result_ != VK_SUCCESS) \
	{ \
		m_logger->WriteError(LogCategory::Vulkan, #call " failed, error was: 0x%08x", (int)_result_); \
		return value; \
	} \
} 
class VulkanGraphics 
	: public IGraphics
	, public std::enable_shared_from_this<VulkanGraphics>
{
private:
	static const Array<const char*> InstanceExtensionsToRequest;
	static const Array<const char*> InstanceLayersToRequest;

	static const Array<const char*> DeviceExtensionsToRequest;
	static const Array<const char*> DeviceLayersToRequest;

	std::shared_ptr<Logger> m_logger;
	String m_gameName;
	int m_gameVersionMajor;
	int m_gameVersionMinor;
	int m_gameVersionBuild;

	GraphicsPresentMode m_preferedPresentMode;

	Array<VkExtensionProperties> m_instanceExtensions;
	Array<VkLayerProperties> m_instanceValidationLayers;

	VkDebugReportCallbackEXT m_debugCallbackHandle;

	VkInstance m_instance;
	VulkanExtensionInfo m_extensionInfo;
	VulkanDeviceInfo m_physicalDeviceInfo;
	VkDevice m_logicalDevice;
	VkSurfaceKHR m_windowSurface;
	VkSwapchainKHR m_swapChain;

	Array<std::shared_ptr<VulkanImage>> m_swapChainImages;
	Array<std::shared_ptr<VulkanImageView>> m_swapChainImageViews;

	VkSurfaceFormatKHR m_swapChainFormat;
	VkPresentModeKHR m_presentMode;

	Array<VkSemaphore> m_imageAvailableSemaphores;
	Array<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_frameFences;
	int m_currentFrame;

	VkQueue m_graphicsQueue;
	VkQueue m_computeQueue;
	VkQueue m_copyQueue;
	VkQueue m_presentQueue;

	VkExtent2D m_swapChainExtent;
	bool m_swapChainRegeneratedThisFrame;

	std::shared_ptr<IWindow> m_window;

	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;

	// todo: change to weak pointers, or better yet check if they are unique, if they are wait
	// x number of frames (how many frames behind we are), before disposing to ensure gpu is finished
	// with them.
	std::mutex m_resourcesMutex;

	Array<std::shared_ptr<VulkanShader>> m_shaders;
	Array<std::shared_ptr<VulkanRenderPass>> m_renderPasses;
	Array<std::shared_ptr<VulkanPipeline>> m_pipelines;
	Array<std::shared_ptr<VulkanFramebuffer>> m_framebuffers;
	Array<std::shared_ptr<VulkanImageView>> m_imageViews;
	Array<std::shared_ptr<VulkanCommandBufferPool>> m_commandBufferPools;	
	Array<std::shared_ptr<VulkanVertexBuffer>> m_vertexBuffers;
	Array<std::shared_ptr<VulkanIndexBuffer>> m_indexBuffers;
	Array<std::shared_ptr<VulkanUniformBuffer>> m_uniformBuffers;
	Array<std::shared_ptr<VulkanResourceSetPool>> m_resourceSetPools;
	Array<std::shared_ptr<VulkanImage>> m_images;
	Array<std::shared_ptr<VulkanSampler>> m_samplers;

	Array<std::shared_ptr<VulkanCommandBuffer>> m_pendingCommandBuffers;
	
	bool Setup();

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg);

	static VKAPI_ATTR VkBool32 VKAPI_CALL StaticDebugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData)
	{
		return reinterpret_cast<VulkanGraphics*>(userData)->DebugCallback(
			flags,
			objType,
			obj,
			location,
			code,
			layerPrefix,
			msg
		);
	}

	bool RecreateSwapChain();
	void DisposeSwapChain();
	bool CreateFullSwapchain();

	bool CreateInstance();
	bool CreateDebugCallback();
	bool ChoosePhysicalDevice();
	bool CreateLogicalDevice();
	bool CreateSurface();
	bool ChooseSwapChainFormat();
	bool ChoosePresentMode();
	bool ChooseSwapChainExtent();
	bool CreateSwapChain();
	bool CreateMemoryAllocator();

	bool CreateSyncObjects();

	void CancelPresent();

public:
	VulkanGraphics(
		std::shared_ptr<Logger> logger, 
		const String& gameName,
		int gameVersionMajor, 
		int gameVersionMinor, 
		int gameVersionBuild);

	// IGraphics

	virtual ~VulkanGraphics();
	virtual void Dispose();

	virtual bool AttachToWindow(std::shared_ptr<IWindow> window);
	virtual void SetPresentMode(GraphicsPresentMode mode);
	virtual bool Present();

	virtual std::shared_ptr<IGraphicsShader> CreateShader(const String& name, const String& entryPoint, GraphicsPipelineStage stage, const Array<char>& data);
	virtual std::shared_ptr<IGraphicsRenderPass> CreateRenderPass(const String& name, const GraphicsRenderPassSettings& settings);
	virtual std::shared_ptr<IGraphicsPipeline> CreatePipeline(const String& name, const GraphicsPipelineSettings& settings);
	virtual std::shared_ptr<IGraphicsFramebuffer> CreateFramebuffer(const String& name, const GraphicsFramebufferSettings& settings);
	virtual std::shared_ptr<IGraphicsImageView> CreateImageView(const String& name, std::shared_ptr<IGraphicsImage> image);
	virtual std::shared_ptr<IGraphicsVertexBuffer> CreateVertexBuffer(const String& name, const VertexBufferBindingDescription& binding, int vertexCount);
	virtual std::shared_ptr<IGraphicsIndexBuffer> CreateIndexBuffer(const String& name, int indexSize, int indexCount);
	virtual std::shared_ptr<IGraphicsUniformBuffer> CreateUniformBuffer(const String& name, int dataSize);
	virtual std::shared_ptr<IGraphicsResourceSetPool> CreateResourceSetPool(const String& name);
	virtual std::shared_ptr<IGraphicsCommandBufferPool> CreateCommandBufferPool(const String& name);
	virtual std::shared_ptr<IGraphicsImage> CreateImage(const String& name, int width, int height, GraphicsFormat format, bool generateMips);
	virtual std::shared_ptr<IGraphicsSampler> CreateSampler(const String& name, const SamplerDescription& settings);

	virtual void Dispatch(std::shared_ptr<IGraphicsCommandBuffer> buffer);

	virtual GraphicsFormat GetSwapChainFormat();
	virtual Array<std::shared_ptr<IGraphicsImageView>> GetSwapChainViews();

	// End IGraphics

	const VulkanExtensionInfo& GetExtensionInfo() const;

	VkInstance GetInstance() const;

	static std::shared_ptr<IGraphics> Create(
		std::shared_ptr<Logger> logger, 
		const String& gameName, 
		int gameVersionMajor, 
		int gameVersionMinor, 
		int gameVersionBuild);
};