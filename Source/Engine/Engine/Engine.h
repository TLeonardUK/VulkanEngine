#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Math.h"

#include "Engine/Graphics/Graphics.h"

#include "Engine/Rendering/Renderer.h"

#include <memory>
#include <chrono>

class IPlatform;
class IWindow;
class IGraphics;
class IInput;
class Logger;
class IGameInstance;
class ResourceManager;

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
	std::shared_ptr<IInput> m_input;
	std::shared_ptr<Logger> m_logger;

	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<ResourceManager> m_resourceManager;

	bool InitPlatform();
	bool InitLogger();
	bool InitWindow();
	bool InitGraphics();
	bool InitInput();
	bool InitRenderer();
	bool InitResourceManager();

	void TermPlatform();
	void TermLogger();
	void TermWindow();
	void TermGraphics();
	void TermInput();
	void TermRenderer();
	void TermResourceManager();

	void UpdateFps();

	bool Init();
	bool Term();
	void MainLoop();

	std::chrono::high_resolution_clock::time_point m_lastFrameTime;
	std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
	float m_fpsCounter;
	float m_frameTimeSumCounter;

	std::shared_ptr<IGameInstance> m_gameInstance;

public:
	bool Run(std::shared_ptr<IGameInstance> gameInstance);

	std::shared_ptr<ResourceManager> GetResourceManager();

};