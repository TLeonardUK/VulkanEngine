#include "Game/Game/VulkanGameInstance.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/TextureResourceLoader.h"

VulkanGameInstance::VulkanGameInstance(std::shared_ptr<Engine> engine)
	: IGameInstance(engine)
{
}

String VulkanGameInstance::GetAssetFolder()
{
	return "../Assets/";
}

String VulkanGameInstance::GetGameName()
{
	return "Vulkan Test";
}

void VulkanGameInstance::GetGameVersion(int& major, int& minor, int& build)
{
	major = 1;
	minor = 0;
	build = 0;
}

PlatformSystem VulkanGameInstance::GetPlatformSystem()
{
	return PlatformSystem::Sdl;
}

GraphicsSystem VulkanGameInstance::GetGraphicsSystem()
{
	return GraphicsSystem::Vulkan;
}

WindowSystem VulkanGameInstance::GetWindowSystem()
{
	return WindowSystem::Sdl;
}

InputSystem VulkanGameInstance::GetInputSystem()
{
	return InputSystem::Sdl;
}

void VulkanGameInstance::GetWindowSettings(int& width, int& height, int& rate, WindowMode& mode)
{
	width = 800;
	height = 600;
	rate = 60;
	mode = WindowMode::Windowed;
}

void VulkanGameInstance::Initialize()
{
	std::shared_ptr<Engine> engine = GetEngine();
	std::shared_ptr<ResourceManager> resourceManager = engine->GetResourceManager();

	ResourcePtr<IResource> texture1 = resourceManager->LoadTypeLess("Game/Textures/chalet.json");
	ResourcePtr<Texture> texture2 = resourceManager->Load<Texture>("Game/Textures/chalet.json");

	texture1.WaitUntilLoaded();
	texture2.WaitUntilLoaded();
}

void VulkanGameInstance::Terminate()
{
}

void VulkanGameInstance::Tick()
{
}