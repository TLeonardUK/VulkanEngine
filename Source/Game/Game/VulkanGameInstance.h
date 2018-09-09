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
	Vector3 m_viewPosition;
	Quaternion m_viewRotation;
	Vector3 m_viewRotationEuler;
	int m_frameCount;

	Entity m_camera;
	Entity m_camera2;
	Entity m_skybox;
	Entity m_environment1;
	Entity m_environment2;
	Entity m_directionalLight;
	Entity m_directionalLight2;
	Entity m_spotLight;
	Entity m_pointLight;

public:
	VulkanGameInstance(std::shared_ptr<Engine> engine);

	virtual String GetAssetFolder();
	virtual String GetCompiledAssetFolder();
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
