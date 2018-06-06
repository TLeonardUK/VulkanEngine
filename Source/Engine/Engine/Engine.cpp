#include "Engine/Build.h"

#include "Engine/Engine/Engine.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Windowing/Window.h"
#include "Engine/Streaming/File.h"

#if defined(USE_SDL_PLATFORM)
#include "Engine/Platform/Sdl/SdlPlatform.h"
#endif

#if defined(USE_SDL_WINDOW)
#include "Engine/Windowing/Sdl/SdlWindow.h"
#endif

#if defined(USE_VULKAN_GRAPHICS)
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#endif

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsRenderPass.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// TODO:
//	Depth buffer needs transition before use.
//	Add some kind of way of doing one-shot comments. Maybe EnqueueSetupCommand(lambda ...);

bool Engine::Init()
{
	if (!InitPlatform())
	{
		return false;
	}
	if (!InitLogger())
	{
		return false;
	}
	if (!InitGraphics())
	{
		return false;
	}
	if (!InitWindow())
	{
		return false;
	}

	return true;
}

bool Engine::Term()
{
	TermWindow();
	TermGraphics();
	TermLogger();
	TermPlatform();

	return true;
}

bool Engine::Run()
{
	bool bResult = true;

	if (Init())
	{
		MainLoop();
	}
	else
	{
		bResult = false;
	}

	Term();

	return bResult;
}

bool Engine::InitLogger()
{
	m_logger = Logger::Create(m_platform);
	if (m_logger == nullptr)
	{
		return false;
	}

	m_logger->WriteInfo(LogCategory::Engine, "Logging initialized.");
	m_platform->SetLogger(m_logger);

	return true;
}

void Engine::TermLogger()
{
	if (m_logger != nullptr)
	{
		m_logger->Dispose();
		m_logger = nullptr;
	}
}

bool Engine::InitPlatform()
{
#if defined(USE_SDL_PLATFORM)
	m_platform = SdlPlatform::Create();
#else
	#error No platform system defined
#endif

	if (m_platform == nullptr)
	{
		return false;
	}

	return true;
}

void Engine::TermPlatform()
{
	if (m_platform != nullptr)
	{
		m_platform->Dispose();
		m_platform = nullptr;
	}
}

bool Engine::InitWindow()
{
#if defined(USE_SDL_WINDOW)
	m_window = SdlWindow::Create(m_logger, m_graphics, m_name, 800, 600, 60, WindowMode::Windowed);
#else
	#error No window system defined
#endif

	if (m_window == nullptr)
	{
		return false;
	}

	m_graphics->SetPresentMode(GraphicsPresentMode::Immediate);

	if (!m_graphics->AttachToWindow(m_window))
	{
		return false;
	}

	return true;
}

void Engine::TermWindow()
{
	if (m_window != nullptr)
	{
		m_window->Dispose();
		m_window = nullptr;
	}
}

bool Engine::InitGraphics()
{
#if defined(USE_VULKAN_GRAPHICS)
	m_graphics = VulkanGraphics::Create(m_logger, m_name, m_assetFolder, m_versionMajor, m_versionMinor, m_versionBuild);
#else
#error No graphics system defined
#endif

	if (m_graphics == nullptr)
	{
		return false;
	}

	return true;
}

void Engine::TermGraphics()
{
	if (m_graphics != nullptr)
	{
		m_graphics->Dispose();
		m_graphics = nullptr;
	}
}

void Engine::CreateResources()
{
	String fragShaderPath = String::Format("%s/Shaders/triangle.frag", m_assetFolder.c_str());
	String vertShaderPath = String::Format("%s/Shaders/triangle.vert", m_assetFolder.c_str());
	String texturePath = String::Format("%s/Models/chalet.jpg", m_assetFolder.c_str());
	String modelPath = String::Format("%s/Models/chalet.obj", m_assetFolder.c_str());

	m_fragShader = m_graphics->CreateShader("Triangle Fragment Shader", "main", GraphicsPipelineStage::Fragment, File::ReadAllBytes(fragShaderPath));
	m_vertShader = m_graphics->CreateShader("Triangle Vertex Shader", "main", GraphicsPipelineStage::Vertex, File::ReadAllBytes(vertShaderPath));

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	m_image = m_graphics->CreateImage("Main Image", texWidth, texHeight, 1, GraphicsFormat::UNORM_R8G8B8A8);
	m_image->Stage(pixels, 0, texWidth * texHeight * 4);
	m_imageView = m_graphics->CreateImageView("Main Image View", m_image);

	SamplerDescription description;
	m_sampler = m_graphics->CreateSampler("Main Image Sampler", description);

	m_commandBufferPool = m_graphics->CreateCommandBufferPool("Main Command Buffer Pool");
	m_resourceSetPool = m_graphics->CreateResourceSetPool("Main Resource Set Pool");
	

	m_vertices.reserve(1000000);
	m_indices.reserve(1000000);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str())) 
	{
		assert(false);
	}
	
	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			m_vertices.push_back(vertex);
			m_indices.push_back(m_indices.size());
		}
	}

	m_hasUploadedState = false;


	m_vertexBufferFormat = {};
	m_vertexBufferFormat.AddAttribute("position", 0, GraphicsBindingFormat::Float3, sizeof(Vertex), offsetof(Vertex, pos));
	m_vertexBufferFormat.AddAttribute("color", 1, GraphicsBindingFormat::Float3, sizeof(Vertex), offsetof(Vertex, color));
	m_vertexBufferFormat.AddAttribute("texCoord", 2, GraphicsBindingFormat::Float2, sizeof(Vertex), offsetof(Vertex, texCoord));
	m_vertexBufferFormat.SetVertexSize(sizeof(Vertex));

	m_indexBuffer = m_graphics->CreateIndexBuffer("Main Index Buffer", sizeof(uint32_t), (int)m_indices.size());
	m_indexBuffer->Stage((void*)m_indices.data(), 0, (int)m_indices.size() * sizeof(uint32_t));

	m_vertexBuffer = m_graphics->CreateVertexBuffer("Main Vertex Buffer", m_vertexBufferFormat, (int)m_vertices.size());
	m_vertexBuffer->Stage((void*)m_vertices.data(), 0, (int)m_vertices.size() * sizeof(Vertex));

	m_resourceSetDescription = {};
	m_resourceSetDescription.AddBinding("ubo", 0, GraphicsBindingType::UniformBufferObject);
	m_resourceSetDescription.AddBinding("sampler", 1, GraphicsBindingType::Sampler);

	m_resourceSet = m_resourceSetPool->Allocate(m_resourceSetDescription);

	m_uniformBuffer = m_graphics->CreateUniformBuffer("Main Uniform Buffer Object", sizeof(m_uniforms));

	CreateSwapChainDependentResources();
}

void Engine::FreeResources()
{
	m_fragShader = nullptr;
	m_vertShader = nullptr;

	m_resourceSet = nullptr;

	FreeSwapChainDependentResources();
}

void Engine::FreeSwapChainDependentResources()
{
	m_depthBufferImage = nullptr;
	m_depthBufferView = nullptr;
	m_renderPass = nullptr;
	m_swapChainFramebuffers.clear();
	m_pipeline = nullptr;
	m_commandBuffers.clear();
}

void Engine::CreateSwapChainDependentResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.AddColorAttachment(m_graphics->GetSwapChainFormat(), true);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_renderPass = m_graphics->CreateRenderPass("Main Render Pass", renderPassSettings);

	m_swapChainViews = m_graphics->GetSwapChainViews();
	m_swapChainWidth = m_swapChainViews[0]->GetWidth();
	m_swapChainHeight = m_swapChainViews[0]->GetHeight();

	// Create depth buffer.
	m_depthBufferImage = m_graphics->CreateImage("Depth Buffer", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_D24_UINT_S8);
	m_depthBufferView = m_graphics->CreateImageView("Depth BUffer View", m_depthBufferImage);

	// Create frame buffers for each swap chain image.
	m_swapChainFramebuffers.resize(m_swapChainViews.size());
	for (int i = 0; i < m_swapChainViews.size(); i++)
	{
		std::shared_ptr<IGraphicsImageView> imageView = m_swapChainViews[i];

		GraphicsFramebufferSettings frameBufferSettings;
		frameBufferSettings.width = imageView->GetWidth();
		frameBufferSettings.height = imageView->GetHeight();
		frameBufferSettings.renderPass = m_renderPass;
		frameBufferSettings.attachments.push_back(imageView);
		frameBufferSettings.attachments.push_back(m_depthBufferView);

		m_swapChainFramebuffers[i] = m_graphics->CreateFramebuffer(String::Format("Swap Chain Framebuffer %i", i), frameBufferSettings);;
	}

	// Create pipeline.
	GraphicsPipelineSettings pipelineSettings;
	pipelineSettings.SetRenderPass(m_renderPass);
	pipelineSettings.SetShaderStage(GraphicsPipelineStage::Vertex, m_vertShader);
	pipelineSettings.SetShaderStage(GraphicsPipelineStage::Fragment, m_fragShader);
	pipelineSettings.SetVertexFormat(m_vertexBufferFormat);
	pipelineSettings.AddResourceSet(m_resourceSet);
	pipelineSettings.SetDepthTestEnabled(true);
	pipelineSettings.SetDepthWriteEnabled(true);

	m_pipeline = m_graphics->CreatePipeline("Main Pipeline", pipelineSettings);

	// Create command buffer.
	m_commandBuffers.resize(m_swapChainViews.size());
	for (int i = 0; i < m_commandBuffers.size(); i++)
	{
		m_commandBuffers[i] = m_commandBufferPool->Allocate();
	}
}

void Engine::SwapChainModified()
{
	FreeSwapChainDependentResources();
	CreateSwapChainDependentResources();
}

void Engine::BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	buffer->Reset();

	buffer->Begin();

	// Upload vertex buffer (todo: only if changed).
	// todo: we shouldn't be doing this, we are meant to wait before buffer is not being used
	// before pushing stuff to it. Use staged data until gpu is ready? Ring buffer? Also be nice
	// if we can get rid of this and do it in another way rather than manual uploads. Just have an Update
	// function of the index/vert buffer and do it automagically?
	if (!m_hasUploadedState)
	{
		buffer->Upload(m_image);
		buffer->Upload(m_indexBuffer);
		buffer->Upload(m_vertexBuffer);
		m_hasUploadedState = true;
	}

	buffer->Clear(m_swapChainViews[m_frameIndex]->GetImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
	buffer->Clear(m_depthBufferImage, Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);

	// Render the frame.
	buffer->BeginPass(m_renderPass, m_swapChainFramebuffers[m_frameIndex]);
	buffer->BeginSubPass();

	buffer->SetPipeline(m_pipeline);
	buffer->SetScissor(0, 0, m_swapChainViews[0]->GetWidth(), m_swapChainViews[0]->GetHeight());
	buffer->SetViewport(0, 0, m_swapChainViews[0]->GetWidth(), m_swapChainViews[0]->GetHeight());

	buffer->SetIndexBuffer(m_indexBuffer);
	buffer->SetVertexBuffer(m_vertexBuffer);
	buffer->SetResourceSets({ m_resourceSet });
	buffer->DrawIndexedElements(m_indices.size(), 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();

	buffer->End();
}

void Engine::UpdateUniforms()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() / 5.0f;

	m_uniforms.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_uniforms.view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_uniforms.proj = glm::perspective(glm::radians(45.0f), m_swapChainWidth / (float)m_swapChainHeight, 0.1f, 10.0f);
	m_uniforms.proj[1][1] *= -1;

	m_uniformBuffer->Upload(&m_uniforms, 0, sizeof(m_uniforms));

	m_resourceSet->UpdateBinding(0, 0, m_uniformBuffer);
	m_resourceSet->UpdateBinding(1, 0, m_sampler, m_imageView);
}

void Engine::UpdateFps()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_lastFrameTime).count();
	m_lastFrameTime = std::chrono::high_resolution_clock::now();

	m_frameTimeSumCounter += time;
	m_fpsCounter++;

	float timeSinceLastUpdate = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_lastUpdateTime).count();
	if (timeSinceLastUpdate >= 1000.0f)
	{
		float avgFrameTime = m_frameTimeSumCounter / m_fpsCounter;

		m_window->SetTitle(String::Format("Vulkan Test (%i fps, %0.2f ms)", (int)m_fpsCounter, avgFrameTime));

		m_fpsCounter = 0.0f;
		m_frameTimeSumCounter = 0.0f;

		m_lastUpdateTime = std::chrono::high_resolution_clock::now();
	}
}

void Engine::MainLoop()
{
	CreateResources();

	m_frameIndex = 0;

	while (!m_platform->WasCloseRequested())
	{
		m_platform->PumpMessageQueue();

		UpdateFps();
		UpdateUniforms();

		std::shared_ptr<IGraphicsCommandBuffer> commandBuffer = m_commandBuffers[m_frameIndex];
		BuildCommandBuffer(commandBuffer);
		m_graphics->Dispatch(commandBuffer);

		m_frameIndex = (m_frameIndex + 1) % m_commandBuffers.size();

		if (m_graphics->Present())
		{
			SwapChainModified();
		}
	}
}