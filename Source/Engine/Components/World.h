#pragma once

typedef unsigned int Entity;

struct TransformComponent
{
	Vector3 Position;
	Quaternion Rotation;
};

struct CameraComponent
{
	float FOV;
	Entity TargetEntity;
};

class DebugCameraControlSystem
{
	void UpdateComponent(CameraComponent& state);
};

class World
{
private:
	Array<Entity> m_entities;

public:

	void CreateEntity();
	void DestroyEntity();

	void IsEntityAlive();

	void CreateComponent();
	void DestroyComponent();

	void AttachComponent();
	void DetachComponent();

	void GetEntityComponents();

	template <typename T>
	void GetEntityComponentsByType();

	void GetEntities();
	void GetEntitiesInRange();

	void Tick();

};

// Always updating data a state from the previous frame, allows parallel updating easily.


