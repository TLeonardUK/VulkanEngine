#include "Game/Game/VulkanGameInstance.h"
#include "Engine/Input/Input.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/TextureResourceLoader.h"
#include "Engine/Resources/Types/Model.h"

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
	width = 960;
	height = 720;
	rate = 60;
	mode = WindowMode::Windowed;
}

void VulkanGameInstance::Initialize()
{
	std::shared_ptr<Engine> engine = GetEngine();
	std::shared_ptr<ResourceManager> resourceManager = engine->GetResourceManager();
	std::shared_ptr<Renderer> renderer = engine->GetRenderer();

	//ResourcePtr<Model> model = resourceManager->Load<Model>("Engine/Models/Sponza/sponza.json");
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/10-0 Depths.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/10-1 Undead Burg.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/13-2 Ash Lake.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/15-0 Sens Fortress.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/15-1 Anor Londo.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/10-2 Firelink Shrine.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/11-0 Painted World of Ariamis.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/12-0 Darkroot Garden+Basin.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/12-1 Oolacile.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/13-0 Catacombs.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/13-1 Tomb of the Giants.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/14-0 Blighttown+Quelaags Domain.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/14-1 Demon Ruins+Lost Izalith.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/16-0 New Londo Ruins+Valley of Drakes.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/17-0 Duke's Archive+Crystal Caves.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/18-0 Kiln of the first Flame.json");
		renderer->TmpAddModelToRender(model);
	}
	{
		ResourcePtr<Model> model = resourceManager->Load<Model>("Game/Models/Dark Souls/18-1 Undead Asylum.json");
		renderer->TmpAddModelToRender(model);
	}

	m_tmpRenderView = std::make_shared<RenderView>();
	renderer->AddView(m_tmpRenderView);

	m_viewPosition = Vector3(0.0f, 0.0f, -10.0f);
	m_viewRotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	m_viewRotationEuler = Vector3(0.0f, 0.0f, 0.0f);
	m_frameCount = 0;
}

void VulkanGameInstance::Terminate()
{
}

void VulkanGameInstance::Tick(const FrameTime& time)
{
	std::shared_ptr<Engine> engine = GetEngine();
	std::shared_ptr<Renderer> renderer = engine->GetRenderer();
	std::shared_ptr<IInput> input = engine->GetInput();

	float swapWidth = renderer->GetSwapChainWidth();
	float swapHeight = renderer->GetSwapChainHeight();
	
	Vector3 forward(0.0f, 0.0f, 1.0f);
	Vector3 up(0.0f, 1.0f, 0.0f);
	Vector3 right(1.0f, 0.0f, 0.0f);

	Matrix4 viewMatrix = glm::lookAt(m_viewPosition, m_viewPosition + (forward * m_viewRotation), -up); // negative to correct for vulkans flipped y.
	Matrix4 projectionMatrix = glm::perspective(glm::radians(65.0f), swapWidth / swapHeight, 0.1f, 10000.0f);

	Vector2 center(swapWidth / 2, swapHeight / 2);
	Vector2 mouse = input->GetMousePosition();
	Vector2 mouseDelta = (mouse - center);

	if (m_frameCount > 5)
	{
		m_viewRotationEuler.y += (-mouseDelta.x / MouseSensitivity);
		m_viewRotationEuler.x = std::min(std::max(m_viewRotationEuler.x - (mouseDelta.y / MouseSensitivity), MATH_PI * -0.4f), MATH_PI * 0.4f);

		Quaternion xRotation = glm::angleAxis(m_viewRotationEuler.y, up);
		Quaternion yRotation = glm::angleAxis(m_viewRotationEuler.x, right);
		m_viewRotation = yRotation * xRotation;
	}

	input->SetMousePosition(center);
	m_frameCount++;

	float speed = MovementSpeed;
	if (input->IsKeyDown(InputKey::Space))
	{
		speed *= 15.0f;
	}

	if (input->IsKeyDown(InputKey::W))
	{
		m_viewPosition += (forward * m_viewRotation) * speed * time.DeltaTime;
	}
	if (input->IsKeyDown(InputKey::S))
	{
		m_viewPosition -= (forward * m_viewRotation) * speed * time.DeltaTime;
	}
	if (input->IsKeyDown(InputKey::A))
	{
		m_viewPosition -= (right * m_viewRotation) * speed * time.DeltaTime;
	}
	if (input->IsKeyDown(InputKey::D))
	{
		m_viewPosition += (right * m_viewRotation) * speed * time.DeltaTime;
	}
	if (input->IsKeyDown(InputKey::Q))
	{
		m_viewPosition += (up * m_viewRotation) * speed * time.DeltaTime;
	}
	if (input->IsKeyDown(InputKey::E))
	{
		m_viewPosition -= (up * m_viewRotation) * speed * time.DeltaTime;
	}

	m_tmpRenderView->Viewport = Rectangle(0.0f, 0.0f, swapWidth, swapHeight);
	m_tmpRenderView->ViewMatrix = viewMatrix;
	m_tmpRenderView->ProjectionMatrix = projectionMatrix;
}