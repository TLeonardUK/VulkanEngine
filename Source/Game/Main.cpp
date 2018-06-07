#include "Engine/Engine/Engine.h"

#include "Game/VulkanGameInstance.h"

int main(int argc, char* argv[])
{
	std::shared_ptr<Engine> engine = std::make_shared<Engine>();
	std::shared_ptr<VulkanGameInstance> vulkanInstance = std::make_shared<VulkanGameInstance>(engine);
	return engine->Run(vulkanInstance) ? 0 : 1;
}