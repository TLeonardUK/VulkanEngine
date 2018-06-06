#include "Engine/Engine.h"

int main(int argc, char* argv[])
{
	Engine engine;
	engine.SetName("Vulkan Test");
	engine.SetVersion(1, 0, 0);
	engine.SetAssetFolder("../Assets");
	//engine.SetPlatformSubSystem(PlatformSubSystem::Sdl);
	//engine.SetWindowSubSystem(WindowSubSystem::Sdl);
	//engine.SetGraphicsSubSystem(GraphicsSubSystem::Vulkan);
	return engine.Run() ? 0 : 1;
}