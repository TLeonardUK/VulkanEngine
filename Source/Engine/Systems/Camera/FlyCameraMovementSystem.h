#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Camera/FlyCameraComponent.h"
#include "Engine/ECS/System.h"

class Renderer;
class IInput;
class ImguiManager;

// Moves a fly camera based on user input. 
//   WASD  to move forward/backwards/left/right
//   QE    to move up/down.
//   Space to speed up.
class FlyCameraMovementSystem
	: public System<FlyCameraComponent, const TransformComponent>
{
private:
	std::shared_ptr<IInput> m_input;
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<ImguiManager> m_imguiManager;

private:

public:
	FlyCameraMovementSystem(
		std::shared_ptr<IInput> input, 
		std::shared_ptr<Renderer> renderer, 
		std::shared_ptr<ImguiManager> imguiManager);

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<FlyCameraComponent*>& behaviours,
		const Array<const TransformComponent*>& transforms);
};
