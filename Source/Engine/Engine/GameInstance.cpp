#include "Pch.h"

#include "Engine/Engine/GameInstance.h"

IGameInstance::IGameInstance(std::shared_ptr<Engine> engine)
	: m_engine(engine)
{
}

IGameInstance::~IGameInstance()
{
}

std::shared_ptr<Engine> IGameInstance::GetEngine()
{
	return m_engine;
}