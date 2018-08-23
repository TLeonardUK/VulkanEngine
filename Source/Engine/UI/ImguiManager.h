#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceManager.h"
//#include "Engine/Resources/Types/Material.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Types/Array.h"

#include "Engine/ThirdParty/imgui/imgui.h"

class Material;
class Logger;
class IGraphics;
class IWindow;
class IGraphicsIndexBuffer;
class IGraphicsVertexBuffer;
class IGraphicsImageView;
struct VertexBufferBindingDescription;
class MeshRenderState;
class Renderer;
class IInput;
class Texture;

struct SDL_Window;

typedef std::function<void()> ImguiCallbackFunction_t;
typedef int ImguiCallbackToken;

enum class ImguiCallback
{
	MainMenu,
	PostMainMenu,
};

class ImguiManager
{
private:
	struct Callback
	{
		ImguiCallbackToken token;
		ImguiCallback type;
		ImguiCallbackFunction_t function;
	};

	struct StoredImage
	{
		std::shared_ptr<IGraphicsImageView> view;
		std::shared_ptr<MeshRenderState> renderData;
	};

	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<IInput> m_input;
	std::shared_ptr<IWindow> m_window;
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<ResourceManager> m_resourceManager;
		
	ResourcePtr<Material> m_material;
	std::shared_ptr<MeshRenderState> m_MeshRenderState;

	std::shared_ptr<IGraphicsIndexBuffer> m_indexBuffer;
	std::shared_ptr<IGraphicsVertexBuffer> m_vertexBuffer;

	Array<Callback> m_callbacks;
	int m_callbackId;

	ResourcePtr<Texture> m_fontTexture;
	Dictionary<ImTextureID, StoredImage> m_storedImages;

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

	void RunCallbacks(ImguiCallback callback);

public:
	ImguiManager(std::shared_ptr<Logger> logger);
	~ImguiManager();

	bool Init(std::shared_ptr<IInput> input, std::shared_ptr<IGraphics> graphics, std::shared_ptr<IWindow> windowing, std::shared_ptr<Renderer> renderer, std::shared_ptr<ResourceManager> resourceManager);
	void Dispose();

	void StartFrame(const FrameTime& time);
	void EndFrame();

	ImTextureID StoreImage(std::shared_ptr<IGraphicsImageView> view);

	void FlagMouseControlRequired();
	bool IsMouseControlRequired();

	ImguiCallbackToken RegisterCallback(ImguiCallback type, ImguiCallbackFunction_t function);
	void UnregisterCallback(ImguiCallbackToken token);

};