#include "Pch.h"

#include "Engine/Build.h"

#include "Engine/Engine/GameInstance.h"
#include "Engine/Engine/Engine.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Windowing/Window.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/TextureResourceLoader.h"
#include "Engine/Resources/Types/TextureCubeResourceLoader.h"
#include "Engine/Resources/Types/ShaderResourceLoader.h"
#include "Engine/Resources/Types/MaterialResourceLoader.h"
#include "Engine/Resources/Types/ModelResourceLoader.h"

#include "Engine/Platform/Sdl/SdlPlatform.h"
#include "Engine/Windowing/Sdl/SdlWindow.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Input/Sdl/SdlInput.h"

#include "Engine/Threading/TaskManager.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsRenderPass.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Camera/CameraViewComponent.h"

#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Mesh/ModelMeshCreationSystem.h"
#include "Engine/Systems/Camera/FlyCameraMovementSystem.h"
#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Render/RenderDirectionalShadowMapSystem.h"

#include "Engine/Utilities/Statistic.h"

#include "Engine/Profiling/Profiling.h"

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
	if (!InitTaskManager())
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
	if (!InitImguiManager())
	{
		return false;
	}
	if (!InitWorld())
	{
		return false;
	}

	return true;
}

bool Engine::Term()
{
	TermWorld();
	TermImguiManager();
	TermResourceManager();
	TermRenderer();
	TermTaskManager();
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
	m_compiledAssetFolder = m_gameInstance->GetCompiledAssetFolder();
	m_gameInstance->GetGameVersion(m_versionMajor, m_versionMinor, m_versionBuild);

	m_startTime = std::chrono::high_resolution_clock::now();
	m_frameCounter = 0;

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
		m_input = SdlInput::Create(m_logger, m_window, m_platform);
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
	m_renderer = std::make_shared<Renderer>(m_logger, m_graphics);

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
	m_resourceManager = std::make_shared<ResourceManager>(m_logger, m_taskManager);
	if (!m_resourceManager->Init())
	{
		return false;
	}

	if (!m_resourceManager->Mount(m_compiledAssetFolder))
	{
		return false;
	}
	if (!m_resourceManager->Mount(m_assetFolder))
	{
		return false;
	}

	m_resourceManager->AddLoader(std::make_shared<TextureResourceLoader>(m_logger, m_graphics, m_renderer));
	m_resourceManager->AddLoader(std::make_shared<TextureCubeResourceLoader>(m_logger, m_graphics, m_renderer));
	m_resourceManager->AddLoader(std::make_shared<ShaderResourceLoader>(m_logger, m_graphics));
	m_resourceManager->AddLoader(std::make_shared<MaterialResourceLoader>(m_logger, m_graphics, m_renderer));
	m_resourceManager->AddLoader(std::make_shared<ModelResourceLoader>(m_logger, m_graphics, m_renderer));
	m_resourceManager->LoadDefaults();

	// Load renderer now resource manager is available.
	if (!m_renderer->Init(m_resourceManager))
	{
		return false;
	}

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

bool Engine::InitImguiManager()
{
	m_imguiManager = std::make_shared<ImguiManager>(m_logger);
	if (!m_imguiManager->Init(m_input, m_graphics, m_window, m_renderer, m_resourceManager))
	{
		return false;
	}

	m_renderer->InitDebugMenus(m_imguiManager);

	return true;
}

void Engine::TermImguiManager()
{
	if (m_imguiManager != nullptr)
	{
		m_imguiManager->Dispose();
		m_imguiManager = nullptr;
	}
}

bool Engine::InitTaskManager()
{
	m_taskManager = std::make_shared<TaskManager>(m_logger);
	TaskManager::AsyncInstance = &*m_taskManager;

	int totalCores = std::thread::hardware_concurrency() - 1; // 1 to take into account the main thread.

	int longQueues = Math::Max(1, totalCores / 4);
	int normalQueues = totalCores - longQueues;

	// We create two types of queues:
	//  - The first type has the most thread workers, and allows running of normal and time-critical tasks. This is where
	//	  most of the frame tasks will get scheduled.
	//  - The second type accepts the same tasks as the previous type, but with the addition of being able to run long-running
	//    tasks (such as resource loads, saves, etc). This only typically has a few thread workers it chokeing the game tasks.

	Dictionary<TaskQueueFlags, int> queues;
	queues[static_cast<TaskQueueFlags>((int)TaskQueueFlags::Normal | (int)TaskQueueFlags::TimeCritical)] = normalQueues;
	queues[static_cast<TaskQueueFlags>((int)TaskQueueFlags::Normal | (int)TaskQueueFlags::TimeCritical | (int)TaskQueueFlags::Long)] = longQueues;

	if (!m_taskManager->Init(queues))
	{
		return false;
	}

	return true;
}

void Engine::TermTaskManager()
{
	if (m_taskManager != nullptr)
	{
		TaskManager::AsyncInstance = nullptr;
		m_taskManager->Dispose();
		m_taskManager = nullptr;
	}
}

bool Engine::InitWorld()
{
	m_world = std::make_shared<World>(m_logger, m_taskManager);
	if (!m_world->Init())
	{
		return false;
	}

	// Camera
	m_world->AddSystem<FlyCameraMovementSystem>(m_input, m_renderer, m_imguiManager);

	// Mesh
	m_world->AddSystem<MeshBoundsUpdateSystem>(m_renderer);
	m_world->AddSystem<ModelMeshCreationSystem>();

	// Render
	m_world->AddSystem<RenderDebugSystem>(m_world, m_renderer, m_resourceManager, m_graphics);
	m_world->AddSystem<RenderCameraViewSystem>(m_world, m_graphics, m_logger, m_renderer);
	m_world->AddSystem<RenderDirectionalShadowMapSystem>(m_world, m_logger, m_renderer, m_resourceManager, m_graphics);

	// Transform
	m_world->AddSystem<TransformSystem>();
	m_world->AddSystem<SpatialIndexSystem>();

	return true;
}

void Engine::TermWorld()
{
	if (m_world != nullptr)
	{
		m_world->Dispose();
		m_world = nullptr;
	}
}

void Engine::UpdateFrameTime()
{
	// Track FPS.
	auto currentTime = std::chrono::high_resolution_clock::now();
	float lastFrameDuration = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_lastFrameTime).count();
	m_lastFrameTime = std::chrono::high_resolution_clock::now();

	m_frameTimeSumCounter += lastFrameDuration;
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

	float timeSinceStart = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_startTime).count();

	// Calculate delta.
	m_frameTime.Time = std::chrono::duration<float, std::chrono::milliseconds::period>(timeSinceStart).count() / 1000.0f;
	m_frameTime.DeltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(lastFrameDuration).count() / 1000.0f;
	m_frameTime.FrameIndex = m_frameCounter++;
}

void Engine::MainLoop()
{
	m_gameInstance->Initialize();

	while (!m_platform->WasCloseRequested())
	{
		ProfileScope scope(ProfileColors::Cpu, "Main Loop");

		// Initial event handling and setup.
		m_imguiManager->StartFrame(m_frameTime);
		Statistic::NextFrame(m_frameTime);
		m_platform->PumpMessageQueue();
		m_input->PollInput();

		// Main frame tick.
		m_gameInstance->Tick(m_frameTime);

		// Tick world.
		m_world->Tick(m_frameTime);

		// Present frame.
		m_imguiManager->EndFrame();
		m_renderer->Present();

		// Collect garbage and track stats.
		m_resourceManager->CollectGarbage();
		m_resourceManager->ProcessPendingLoads();
		UpdateFrameTime();
	}

	m_gameInstance->Terminate();
}

std::shared_ptr<ResourceManager> Engine::GetResourceManager()
{
	return m_resourceManager;
}

std::shared_ptr<Renderer> Engine::GetRenderer()
{
	return m_renderer;
}

std::shared_ptr<IInput> Engine::GetInput()
{
	return m_input;
}

std::shared_ptr<ImguiManager> Engine::GetImGuiManager()
{
	return m_imguiManager;
}

std::shared_ptr<World> Engine::GetWorld()
{
	return m_world;
}