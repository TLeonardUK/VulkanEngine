#pragma once

#include <memory>

#include "Engine/Engine/GameInstance.h"
#include "Engine/Engine/Engine.h"

class VulkanGameInstance
	: public IGameInstance
{
protected:

public:
	VulkanGameInstance(std::shared_ptr<Engine> engine);

	virtual String GetAssetFolder();
	virtual String GetGameName();
	virtual void GetGameVersion(int& major, int& minor, int& build);

	virtual PlatformSystem GetPlatformSystem();
	virtual GraphicsSystem GetGraphicsSystem();
	virtual WindowSystem GetWindowSystem();
	virtual InputSystem GetInputSystem();

	virtual void GetWindowSettings(int& width, int& height, int& rate, WindowMode& mode);

	virtual void Initialize();
	virtual void Terminate();
	virtual void Tick();

};
