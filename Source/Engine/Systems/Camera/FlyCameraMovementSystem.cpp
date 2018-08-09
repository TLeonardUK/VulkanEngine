#include "Pch.h"

#include "Engine/Systems/Camera/FlyCameraMovementSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/UI/ImguiManager.h"
#include "Engine/Input/Input.h"

FlyCameraMovementSystem::FlyCameraMovementSystem(std::shared_ptr<IInput> input, std::shared_ptr<Renderer> renderer, std::shared_ptr<ImguiManager> imguiManager)
	: m_input(input)
	, m_renderer(renderer)
	, m_imguiManager(imguiManager)
{
	AddSuccessor<TransformSystem>();

	// make all types const except the one we are working on.
	//
	// world->AddMessageCallback();
	//
	//world->GetMessageQueue();
	//for (auto iter : messageQueue)
	//{
	//    do stuff
	//}

	// world->AddMessageListener<ComponentAddedMessage>();
	// world->PostMessage<SetTransformPosition>();
}

void FlyCameraMovementSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<FlyCameraComponent*>& behaviours,
	const Array<const TransformComponent*>& transforms)
{
	float swapWidth = static_cast<float>(m_renderer->GetSwapChainWidth());
	float swapHeight = static_cast<float>(m_renderer->GetSwapChainHeight());

	bool spaceDown = m_input->IsKeyDown(InputKey::Space);
	bool wDown = m_input->IsKeyDown(InputKey::W);
	bool sDown = m_input->IsKeyDown(InputKey::S);
	bool aDown = m_input->IsKeyDown(InputKey::A);
	bool dDown = m_input->IsKeyDown(InputKey::D);
	bool qDown = m_input->IsKeyDown(InputKey::Q);
	bool eDown = m_input->IsKeyDown(InputKey::E);

	float speedMultiplier = 1.0f;
	if (spaceDown)
	{
		speedMultiplier *= 15.0f;
	}

	bool mouseCaptured = m_imguiManager->IsMouseControlRequired();

	// Consume all relevant messages.
	for (auto& message : world.ConsumeMessages<SetFlyCameraConfigMessage>())
	{
		FlyCameraComponent* component = message.component.Get(world);
		if (component != nullptr)
		{
			component->mouseSensitivity = message.mouseSensitivity;
			component->movementSpeed = message.movementSpeed;
		}
	}

	// Update each camera.
	for (size_t i = 0; i < entities.size(); i++)
	{
		Entity entity = entities[i];
		FlyCameraComponent* behaviour = behaviours[i];
		const TransformComponent* transform = transforms[i];

		Vector3 viewPosition = transform->localPosition;
		Quaternion viewRotation = transform->localRotation;

		float speed = behaviour->movementSpeed * speedMultiplier;
		float sensitivity = behaviour->mouseSensitivity;

		Vector3& rotationEuler = behaviour->viewRotationEuler;

		if (!mouseCaptured)
		{
			Vector2 center(swapWidth / 2, swapHeight / 2);
			Vector2 mouse = m_input->GetMousePosition();
			Vector2 mouseDelta = (mouse - center);

			if (frameTime.FrameIndex > 5)
			{
				rotationEuler.y += (-mouseDelta.x / sensitivity);
				rotationEuler.x = Math::Min(Math::Max(rotationEuler.x - (mouseDelta.y / sensitivity), Math::Pi * -0.4f), Math::Pi * 0.4f);

				Quaternion xRotation = Quaternion::AngleAxis(rotationEuler.y, Vector3::Up);
				Quaternion yRotation = Quaternion::AngleAxis(rotationEuler.x, Vector3::Right);
				viewRotation = yRotation * xRotation;
			}

			m_input->SetMousePosition(center);
			m_input->SetMouseHidden(true);
		}
		else
		{
			m_input->SetMouseHidden(false);
		}

		if (wDown)
		{
			viewPosition += (Vector3::Forward * viewRotation) * speed * frameTime.DeltaTime;
		}
		if (sDown)
		{
			viewPosition -= (Vector3::Forward * viewRotation) * speed * frameTime.DeltaTime;
		}
		if (aDown)
		{
			viewPosition -= (Vector3::Right * viewRotation) * speed * frameTime.DeltaTime;
		}
		if (dDown)
		{
			viewPosition += (Vector3::Right * viewRotation) * speed * frameTime.DeltaTime;
		}
		if (qDown)
		{
			viewPosition += (Vector3::Up * viewRotation) * speed * frameTime.DeltaTime;
		}
		if (eDown)
		{
			viewPosition -= (Vector3::Up * viewRotation) * speed * frameTime.DeltaTime;
		}

		SetTransformMessage msg;
		msg.component = entity;
		msg.localPosition = viewPosition;
		msg.localRotation = viewRotation;
		msg.localScale = transform->localScale;
		world.QueueMessage(msg);
	}
}
