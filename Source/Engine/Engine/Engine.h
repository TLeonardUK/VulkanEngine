#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Math.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/ECS/World.h"

class IPlatform;
class IWindow;
class IGraphics;
class IInput;
class Logger;
class IGameInstance;
class ResourceManager;
class ImguiManager;
class TaskManager;
class World;

class Engine
{
private:
	String m_assetFolder;
	String m_compiledAssetFolder;
	String m_name;
	int m_versionMajor;
	int m_versionMinor;
	int m_versionBuild;

	std::shared_ptr<IPlatform> m_platform;
	std::shared_ptr<IWindow> m_window;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<IInput> m_input;
	std::shared_ptr<Logger> m_logger;

	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<ResourceManager> m_resourceManager;

	std::shared_ptr<ImguiManager> m_imguiManager;
	std::shared_ptr<TaskManager> m_taskManager;

	std::chrono::high_resolution_clock::time_point m_startTime;
	std::chrono::high_resolution_clock::time_point m_lastFrameTime;
	std::chrono::high_resolution_clock::time_point m_lastUpdateTime;

	int m_frameCounter;
	float m_fpsCounter;
	float m_frameTimeSumCounter;

	std::shared_ptr<World> m_world;

	FrameTime m_frameTime;

	std::shared_ptr<IGameInstance> m_gameInstance;

private:
	bool InitPlatform();
	bool InitLogger();
	bool InitWindow();
	bool InitGraphics();
	bool InitInput();
	bool InitRenderer();
	bool InitResourceManager();
	bool InitImguiManager();
	bool InitTaskManager();
	bool InitWorld();

	void TermPlatform();
	void TermLogger();
	void TermWindow();
	void TermGraphics();
	void TermInput();
	void TermRenderer();
	void TermResourceManager();
	void TermImguiManager();
	void TermTaskManager();
	void TermWorld();

	void UpdateFrameTime();

	bool Init();
	bool Term();
	void MainLoop();

public:
	bool Run(std::shared_ptr<IGameInstance> gameInstance);

	std::shared_ptr<ResourceManager> GetResourceManager();
	std::shared_ptr<Renderer> GetRenderer();
	std::shared_ptr<IInput> GetInput();
	std::shared_ptr<ImguiManager> GetImGuiManager();
	std::shared_ptr<World> GetWorld();

};