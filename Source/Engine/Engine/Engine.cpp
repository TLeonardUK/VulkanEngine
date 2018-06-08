#include "Engine/Build.h"

#include "Engine/Engine/GameInstance.h"
#include "Engine/Engine/Engine.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Windowing/Window.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/TextureResourceLoader.h"
#include "Engine/Resources/Types/ShaderResourceLoader.h"
#include "Engine/Resources/Types/MaterialResourceLoader.h"
#include "Engine/Resources/Types/ModelResourceLoader.h"

#include "Engine/Platform/Sdl/SdlPlatform.h"
#include "Engine/Windowing/Sdl/SdlWindow.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Input/Sdl/SdlInput.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsRenderPass.h"

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
	if (!InitInput())
	{
		return false;
	}
	if (!InitRenderer())
	{
		return false;
	}
	if (!InitResourceManager())
	{
		return false;
	}

	return true;
}

bool Engine::Term()
{
	TermResourceManager();
	TermRenderer();
	TermInput();
	TermWindow();
	TermGraphics();
	TermLogger();
	TermPlatform();

	return true;
}

bool Engine::Run(std::shared_ptr<IGameInstance> gameInstance)
{
	bool bResult = true;

	m_gameInstance = gameInstance;
	m_name = m_gameInstance->GetGameName();
	m_assetFolder = m_gameInstance->GetAssetFolder();
	m_gameInstance->GetGameVersion(m_versionMajor, m_versionMinor, m_versionBuild);

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
	PlatformSystem system = m_gameInstance->GetPlatformSystem();
	if (system == PlatformSystem::Sdl)
	{
		m_platform = SdlPlatform::Create();
	}
	else
	{
		return false;
	}

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
	int windowWidth, windowHeight, windowRate;
	WindowMode windowMode;

	m_gameInstance->GetWindowSettings(windowWidth, windowHeight, windowRate, windowMode);

	WindowSystem system = m_gameInstance->GetWindowSystem();
	if (system == WindowSystem::Sdl)
	{
		m_window = SdlWindow::Create(m_logger, m_graphics, m_name, windowWidth, windowHeight, windowRate, windowMode);
	}
	else
	{
		return false;
	}

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
	GraphicsSystem system = m_gameInstance->GetGraphicsSystem();
	if (system == GraphicsSystem::Vulkan)
	{
		m_graphics = VulkanGraphics::Create(m_logger, m_name, m_versionMajor, m_versionMinor, m_versionBuild);
	}
	else
	{
		return false;
	}

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

bool Engine::InitInput()
{
	InputSystem system = m_gameInstance->GetInputSystem();
	if (system == InputSystem::Sdl)
	{
		m_input = SdlInput::Create(m_logger);
	}
	else
	{
		return false;
	}

	if (m_input == nullptr)
	{
		return false;
	}

	return true;
}

void Engine::TermInput()
{
	if (m_input != nullptr)
	{
		m_input->Dispose();
		m_input = nullptr;
	}
}

bool Engine::InitRenderer()
{
	m_renderer = std::make_shared<Renderer>(m_graphics);
	if (!m_renderer->Init())
	{
		return false;
	}

	return true;
}

void Engine::TermRenderer()
{
	if (m_renderer != nullptr)
	{
		m_renderer->Dispose();
		m_renderer = nullptr;
	}
}

bool Engine::InitResourceManager()
{
	m_resourceManager = std::make_shared<ResourceManager>(m_logger);
	if (!m_resourceManager->Init())
	{
		return false;
	}

	if (!m_resourceManager->Mount(m_assetFolder))
	{
		return false;
	}

	m_resourceManager->AddLoader(std::make_shared<TextureResourceLoader>(m_logger, m_graphics));
	m_resourceManager->AddLoader(std::make_shared<ShaderResourceLoader>(m_logger, m_graphics));
	m_resourceManager->AddLoader(std::make_shared<MaterialResourceLoader>(m_logger, m_graphics));
	m_resourceManager->AddLoader(std::make_shared<ModelResourceLoader>(m_logger, m_graphics));
	m_resourceManager->LoadDefaults();

	return true;
}

void Engine::TermResourceManager()
{
	if (m_resourceManager != nullptr)
	{
		m_resourceManager->Dispose();
		m_resourceManager = nullptr;
	}
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

		m_window->SetTitle(StringFormat("%s (%i fps, %0.2f ms)", m_name.c_str(), (int)m_fpsCounter, avgFrameTime));

		m_fpsCounter = 0.0f;
		m_frameTimeSumCounter = 0.0f;

		m_lastUpdateTime = std::chrono::high_resolution_clock::now();
	}
}

void Engine::MainLoop()
{
	m_gameInstance->Initialize();

	while (!m_platform->WasCloseRequested())
	{
		m_platform->PumpMessageQueue();

		m_gameInstance->Tick();
		m_renderer->Present();
		m_resourceManager->CollectGarbage();

		UpdateFps();
	}

	m_gameInstance->Terminate();
}

std::shared_ptr<ResourceManager> Engine::GetResourceManager()
{
	return m_resourceManager;
}