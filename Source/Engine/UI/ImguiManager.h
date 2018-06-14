#pragma once

#include <memory>

#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Engine/FrameTime.h"

class Logger;
class IGraphics;
class IWindow;
class IGraphicsIndexBuffer;
class IGraphicsVertexBuffer;
class Renderer;
struct VertexBufferBindingDescription;
class IInput;

struct SDL_Window;

class ImguiManager
{
private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<IInput> m_input;
	std::shared_ptr<IWindow> m_window;
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<ResourceManager> m_resourceManager;
		
	ResourcePtr<Material> m_material;
	std::shared_ptr<IGraphicsIndexBuffer> m_indexBuffer;
	std::shared_ptr<IGraphicsVertexBuffer> m_vertexBuffer;

	ResourcePtr<Texture> m_fontTexture;

	bool m_mouseRequired;
	bool m_mouseRequiredFlagged;

	bool m_debugMenuActive;

	String m_clipboardText;

private:
	bool LoadFontTextures();

	static const char* GetClipboardTestCallback(void* userdata);
	static void SetClipboardTestCallback(void* userdata, const char* text);

	void UpdateInput(); 

	void UpdateDebugMenu();

public:
	ImguiManager(std::shared_ptr<Logger> logger);
	~ImguiManager();

	bool Init(std::shared_ptr<IInput> input, std::shared_ptr<IGraphics> graphics, std::shared_ptr<IWindow> windowing, std::shared_ptr<Renderer> renderer, std::shared_ptr<ResourceManager> resourceManager);
	void Dispose();

	void StartFrame(const FrameTime& time);
	void EndFrame();

	void FlagMouseControlRequired();
	bool IsMouseControlRequired();

};