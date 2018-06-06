#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"

#include <memory>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class IPlatform;
class IWindow;
class IGraphics;
class Logger;

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

class Engine
{
private:
	String m_assetFolder;
	String m_name;
	int m_versionMajor;
	int m_versionMinor;
	int m_versionBuild;

	std::shared_ptr<IPlatform> m_platform;
	std::shared_ptr<IWindow> m_window;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Logger> m_logger;

	bool InitPlatform();
	bool InitLogger();
	bool InitWindow();
	bool InitGraphics();

	void TermPlatform();
	void TermLogger();
	void TermWindow();
	void TermGraphics();

	void CreateResources();
	void FreeResources();
	void CreateSwapChainDependentResources();
	void FreeSwapChainDependentResources();

	void SwapChainModified();

	void UpdateFps();
	void UpdateUniforms();
	void BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer);

	bool Init();
	bool Term();
	void MainLoop();

	std::shared_ptr<IGraphicsShader> m_fragShader;
	std::shared_ptr<IGraphicsShader> m_vertShader;
	std::shared_ptr<IGraphicsRenderPass> m_renderPass;
	Array<std::shared_ptr<IGraphicsFramebuffer>> m_swapChainFramebuffers;
	std::shared_ptr<IGraphicsPipeline> m_pipeline;
	std::shared_ptr<IGraphicsCommandBufferPool> m_commandBufferPool;
	std::shared_ptr<IGraphicsResourceSetPool> m_resourceSetPool;
	std::vector<std::shared_ptr<IGraphicsCommandBuffer>> m_commandBuffers;
	std::shared_ptr<IGraphicsVertexBuffer> m_vertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> m_indexBuffer;
	std::shared_ptr<IGraphicsUniformBuffer> m_uniformBuffer;
	std::shared_ptr<IGraphicsResourceSet> m_resourceSet;
	Array<std::shared_ptr<IGraphicsImageView>> m_swapChainViews;

	std::shared_ptr<IGraphicsImage> m_depthBufferImage;
	std::shared_ptr<IGraphicsImageView> m_depthBufferView;

	std::shared_ptr<IGraphicsImage> m_image;
	std::shared_ptr<IGraphicsImageView> m_imageView;
	std::shared_ptr<IGraphicsSampler> m_sampler;

	bool m_hasUploadedState;
	Array<Vertex> m_vertices;
	Array<uint32_t> m_indices;

	VertexBufferBindingDescription m_vertexBufferFormat;
	GraphicsResourceSetDescription m_resourceSetDescription;

	UniformBufferObject m_uniforms;

	int m_swapChainWidth;
	int m_swapChainHeight;

	int m_frameIndex;

	std::chrono::high_resolution_clock::time_point m_lastFrameTime;
	std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
	float m_fpsCounter;
	float m_frameTimeSumCounter;


public:
	void SetAssetFolder(const String& path)
	{
		m_assetFolder = path;
	}

	void SetName(const String& name)
	{
		m_name = name;
	}

	void SetVersion(int major, int minor, int build)
	{
		m_versionMajor = major;
		m_versionMinor = minor;
		m_versionBuild = build;
	}

	bool Run();
};