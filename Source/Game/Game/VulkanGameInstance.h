#pragma once

#include <memory>

#include "Engine/Engine/GameInstance.h"
#include "Engine/Engine/Engine.h"

#include "Engine/Types/Math.h"

class RenderView;

class VulkanGameInstance
	: public IGameInstance
{
protected:
	std::shared_ptr<RenderView> m_tmpRenderView;
	Vector3 m_viewPosition;
	Quaternion m_viewRotation;
	Vector3 m_viewRotationEuler;
	int m_frameCount;

	const float MouseSensitivity = 500.0f;
	const float MovementSpeed = 20.0f;

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
	virtual void Tick(const FrameTime& time);

};
