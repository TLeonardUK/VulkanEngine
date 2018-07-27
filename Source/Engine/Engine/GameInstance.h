#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Windowing/Window.h"

class Engine;

enum class PlatformSystem
{
	Sdl
};

enum class GraphicsSystem
{
	Vulkan
};

enum class WindowSystem
{
	Sdl
};

enum class InputSystem
{
	Sdl
};

class IGameInstance
{
private:
	std::shared_ptr<Engine> m_engine;

protected:
	IGameInstance(std::shared_ptr<Engine> engine);

public:
	virtual ~IGameInstance();

	virtual std::shared_ptr<Engine> GetEngine();

	virtual String GetAssetFolder() = 0;
	virtual String GetCompiledAssetFolder() = 0;
	virtual String GetGameName() = 0;
	virtual void GetGameVersion(int& major, int& minor, int& build) = 0;

	virtual PlatformSystem GetPlatformSystem() = 0;
	virtual GraphicsSystem GetGraphicsSystem() = 0;
	virtual WindowSystem GetWindowSystem() = 0;
	virtual InputSystem GetInputSystem() = 0;

	virtual void GetWindowSettings(int& width, int& height, int& rate, WindowMode& mode) = 0;

	virtual void Initialize() = 0;
	virtual void Terminate() = 0;
	virtual void Tick(const FrameTime& time) = 0;

};
