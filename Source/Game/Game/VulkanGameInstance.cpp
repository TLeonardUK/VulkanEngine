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
#include "Engine/Components/Lighting/SpotLightComponent.h"
#include "Engine/Components/Lighting/PointLightComponent.h"

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
	width = 2560;
	height = 1440;
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

	/*{
		m_environment1 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment1);
		rootTransform->localPosition = Vector3(2.0f, 2.0f, 2.0f);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment1);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Bistro/interior.json");
	}*/
	/*{
		m_environment2 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment2);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment2);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Bistro/exterior.json");
	}*/
	/*{
		m_environment2 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment2);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment2);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Bedroom/iscv2.json");
	}*/
	/*{
		m_environment1 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment1);
		rootTransform->localScale = Vector3(100.0f, 100.0f, 100.0f);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment1);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/SanMiguel/san-miguel.json");
	}*/
	{
		m_environment1 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_environment1);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_environment1);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Sponza/sponza.json");
	}

	/*for (int x = -1; x < 3; x++)
	{
		for (int z = -1; z < 3; z++)
		{
			Entity sponzaEntity = world->CreateEntity();

			TransformComponent* rootTransform = world->AddComponent<TransformComponent>(sponzaEntity);
			rootTransform->localPosition = Vector3(x * 5000.0f, 0.0f, z * 5000.0f);

			ModelComponent* modelComponent = world->AddComponent<ModelComponent>(sponzaEntity);
			modelComponent->model = resourceManager->Load<Model>("Engine/Models/Sponza/sponza.json");
		}
	}*/

	// Create skybox.
	{
		m_skybox = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_skybox);

		ModelComponent* modelComponent = world->AddComponent<ModelComponent>(m_skybox);
		modelComponent->model = resourceManager->Load<Model>("Engine/Models/Skyboxes/blue_sky.json");
	}

	// Create directional light.
	/*{
		m_directionalLight = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_directionalLight);
		rootTransform->localPosition = Vector3(2049.83472f, 3228.46704f, -483.595795f);
		rootTransform->localRotation = Quaternion(0.389321238f, -0.530930877f, 0.299298376f, -0.690624118f);

		DirectionalLightComponent* lightComponent = world->AddComponent<DirectionalLightComponent>(m_directionalLight);
		lightComponent->isShadowCasting = true;
		lightComponent->shadowMapCascades = 3;
		lightComponent->shadowMapSize = 1024;
		lightComponent->shadowDistance = 4000.0f;
		lightComponent->shadowMapSplitExponent = 0.95f;// 0.98f;
		lightComponent->shadowMapCascadeBlendFactor = 0.1f;
	}*/
	
	// Create debug fly camera.
	{
		m_camera = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_camera);

		CameraViewComponent* cameraView = world->AddComponent<CameraViewComponent>(m_camera);
		cameraView->depthMin = 10.f;
		cameraView->depthMax = 10000.0f;
		cameraView->fov = 90.0f;
		//cameraView->viewport = Rect(0.0f, 0.0f, 0.5f, 0.5f);
		cameraView->viewport = Rect(0.0f, 0.0f, 1.0f, 1.0f);

		FlyCameraComponent* flyCamera = world->AddComponent<FlyCameraComponent>(m_camera);
		flyCamera->movementSpeed = 50.0f;
		flyCamera->mouseSensitivity = 500.0f;
	}

	// Create spot light.
	/*{
		m_spotLight = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_spotLight);
		rootTransform->localPosition = Vector3(500.0f, 1500.0f, 500.0f);
		rootTransform->localRotation = Quaternion::Identity;
		//rootTransform->parent = m_camera;

		// Hack: We shouldn't be modifiying any of these components directly, we should be using messages ...
		//TransformComponent* cameraTransform = world->GetComponent<TransformComponent>(m_camera);
		//cameraTransform->children.push_back(m_spotLight);

		SpotLightComponent* lightComponent = world->AddComponent<SpotLightComponent>(m_spotLight);
		lightComponent->isShadowCasting = true;
		lightComponent->shadowMapSize = 512;
		lightComponent->angle = 45.0f;
		lightComponent->range = 2500.0f;
	}*/

	// Create point light.
	{
		m_pointLight = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_pointLight);
		rootTransform->localPosition = Vector3(0.0f, 500.0f, 0.0f);// Vector3(500.0f, 1500.0f, 500.0f);
		rootTransform->localRotation = Quaternion::Identity;
		//rootTransform->parent = m_camera;

		// Hack: We shouldn't be modifiying any of these components directly, we should be using messages ...
		//TransformComponent* cameraTransform = world->GetComponent<TransformComponent>(m_camera);
		//cameraTransform->children.push_back(m_pointLight);

		PointLightComponent* lightComponent = world->AddComponent<PointLightComponent>(m_pointLight);
		lightComponent->isShadowCasting = true;
		lightComponent->shadowMapSize = 1024;
		lightComponent->radius = 5000.0f;
	}

	/*{
		m_camera2 = world->CreateEntity();

		TransformComponent* rootTransform = world->AddComponent<TransformComponent>(m_camera2);
		rootTransform->localPosition = Vector3(0.0f, 1000.0f, 0.0f);

		CameraViewComponent* cameraView = world->AddComponent<CameraViewComponent>(m_camera2);
		cameraView->depthMin = 5.0f;
		cameraView->depthMax = 15000.0f;
		cameraView->fov = 65.0f;
		cameraView->viewport = Rect(0.5f, 0.5f, 0.5f, 0.5f);
	}*/


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

	std::shared_ptr<Engine> engine = GetEngine();
	std::shared_ptr<World> world = engine->GetWorld();

	static float angle = 0.2f;
	angle += time.DeltaTime * 2.0f;

	//TransformComponent* rootTransform = world->GetComponent<TransformComponent>(m_directionalLight);
	//rootTransform->localRotation = Quaternion::AngleAxis(Math::Pi*1.25f, Vector3::Right) * Quaternion::AngleAxis(angle, Vector3::Up);
	//rootTransform->isDirty = true;


//	TransformComponent* rootTransform = world->GetComponent<TransformComponent>(m_spotLight);
//	rootTransform->localRotation = Quaternion::AngleAxis(Math::Pi*1.25f, Vector3::Right) * Quaternion::AngleAxis(angle, Vector3::Up);
//	rootTransform->isDirty = true;
}