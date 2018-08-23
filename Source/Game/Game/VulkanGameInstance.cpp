#include "Game/Game/VulkanGameInstance.h"
#include "Engine/Input/Input.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/TextureResourceLoader.h"
#include "Engine/Resources/Types/Model.h"
#include "Engine/UI/ImguiManager.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "Engine/Profiling/Profiling.h"

#include "Engine/Components/Camera/CameraViewComponent.h"
#include "Engine/Components/Camera/FlyCameraComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/Mesh/ModelComponent.h"
#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Lighting/DirectionalLightComponent.h"

VulkanGameInstance::VulkanGameInstance(std::shared_ptr<Engine> engine)
	: IGameInstance(engine)
{
}

String VulkanGameInstance::GetAssetFolder()
{
	return "../Assets/";
}

String VulkanGameInstance::GetCompiledAssetFolder()
{
	return "../Temp/Assets/";
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
	width = 1280;
	height = 960;
	rate = 60;
	mode = WindowMode::Windowed;
}

void VulkanGameInstance::Initialize()
{
	std::shared_ptr<Engine> engine = GetEngine();
	std::shared_ptr<World> world = engine->GetWorld();
	std::shared_ptr<ResourceManager> resourceManager = engine->GetResourceManager();
	std::shared_ptr<Renderer> renderer = engine->GetRenderer();

	{
		//ResourcePtr<Model> model = resourceManager->Load<Model>("Engine/Models/Sponza/sponza.json");
		//renderer->TmpAddModelToRender(model);
	}
	{
		//ResourcePtr<Model> model = resourceManager->Load<Model>("Engine/Models/Skyboxes/blue_sky.json");
		//renderer->TmpAddModelToRender(model);
	}
	{
		//ResourcePtr<Model> model = resourceManager->Load<Model>("Engine/Models/Bistro/exterior.json");
		//renderer->TmpAddModelToRender(model);
	}
	{
		//ResourcePtr<Model> model = resourceManager->Load<Model>("Engine/Models/Bistro/interior.json");
		//renderer->TmpAddModelToRender(model);
	}
	/*{
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
	}*/


	// Create environment.	
	/*for (int x = 0; x < 15; x++) 
	{
		for (int y = 0; y < 15; y++)
		{
			for (int z = 0; z < 15; z++)
			{
				Entity cube = world->CreateEntity();

				TransformComponent* rootTransform = world->AddComponent<TransformComponent>(cube);
				rootTransform->localPosition = Vector3((float)x, (float)y, (float)z) * Vector3(10.0f, 10.0f, 10.0f);
				rootTransform->localScale = Vector3(5.0f, 5.0f, 5.0f);

				ModelComponent* modelComponent = world->AddComponent<ModelComponent>(cube);
				modelComponent->model = resourceManager->Load<Model>("Engine/Models/Default.json");
			}
		}
	}*/

	{
		m_environment1 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment1);
		rootTransform->localPosition = Vector3(2.0f, 2.0f, 2.0f);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment1);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Bistro/interior.json");
	}
	{
		m_environment2 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment2);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment2);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Bistro/exterior.json");
	}
	/*{
		m_environment1 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment1);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment1);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/SanMiguel/san-miguel.json");
	}	
	{
		m_environment1 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment1);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment1);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Sponza/sponza.json");
	}*/

	// Create skybox.
	{
		m_skybox = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_skybox);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_skybox);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Skyboxes/blue_sky.json");
	}

	// Create directional light.
	{
		m_directionalLight = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_directionalLight);

		DirectionalLightComponent* lightComponent = world->AddComponent<DirectionalLightComponent>(m_directionalLight);
		lightComponent->isShadowCasting = true;
	}

	// Create debug fly camera.
	{
		m_camera = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_camera);

		CameraViewComponent* cameraView = world->AddComponent<CameraViewComponent>(m_camera);
		cameraView->depthMin = 5.0f;
		cameraView->depthMax = 15000.0f;
		cameraView->fov = 65.0f;
		cameraView->viewport = Rect(0.0f, 0.0f, 1.0f, 1.0f);

		FlyCameraComponent* flyCamera = world->AddComponent<FlyCameraComponent>(m_camera);
		flyCamera->movementSpeed = 50.0f;
		flyCamera->mouseSensitivity = 500.0f;
	}

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
	ProfileScope scope(ProfileColors::Draw, "VulkanGameInstance::Tick");

}