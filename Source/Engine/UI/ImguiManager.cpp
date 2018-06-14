#include "Engine/UI/ImguiManager.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Rendering/Renderer.h"

#include "Engine/Input/Input.h"

#include "Engine/Resources/Types/TextureResourceLoader.h"

#include "Engine/ThirdParty/imgui/imgui.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Windowing/Sdl/SdlWindow.h"

// todo: replace bindings with ones that interface with engine.

static MaterialPropertyHash ImGuiScale = CalculateMaterialPropertyHash("ImGuiScale");
static MaterialPropertyHash ImGuiTranslation = CalculateMaterialPropertyHash("ImGuiTranslation");
static MaterialPropertyHash ImGuiTexture = CalculateMaterialPropertyHash("ImGuiTexture");

const char* ImguiManager::GetClipboardTestCallback(void* userdata)
{
	ImguiManager* manager = static_cast<ImguiManager*>(userdata);

	manager->m_clipboardText = manager->m_input->GetClipboardText();
	return manager->m_clipboardText.data();
}

void ImguiManager::SetClipboardTestCallback(void* userdata, const char* text)
{
	ImguiManager* manager = static_cast<ImguiManager*>(userdata);

	manager->m_input->SetClipboardText(text);
}

ImguiManager::ImguiManager(std::shared_ptr<Logger> logger)
	: m_logger(logger)
{
}

ImguiManager::~ImguiManager()
{
}

bool ImguiManager::Init(std::shared_ptr<IInput> input, std::shared_ptr<IGraphics> graphics, std::shared_ptr<IWindow> window, std::shared_ptr<Renderer> renderer, std::shared_ptr<ResourceManager> resourceManager)
{
	m_debugMenuActive = false;
	m_mouseRequired = false;
	m_input = input;
	m_window = window;
	m_graphics = graphics;
	m_renderer = renderer;
	m_resourceManager = resourceManager;

	m_material = m_resourceManager->Load<Material>("Engine/Materials/imgui.json");
	m_material.WaitUntilLoaded();
	
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;      
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.KeyMap[ImGuiKey_Tab] = (int)InputKey::Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = (int)InputKey::Left;
	io.KeyMap[ImGuiKey_RightArrow] = (int)InputKey::Right;
	io.KeyMap[ImGuiKey_UpArrow] = (int)InputKey::Up;
	io.KeyMap[ImGuiKey_DownArrow] = (int)InputKey::Down;
	io.KeyMap[ImGuiKey_PageUp] = (int)InputKey::PageUp;
	io.KeyMap[ImGuiKey_PageDown] = (int)InputKey::PageDown;
	io.KeyMap[ImGuiKey_Home] = (int)InputKey::Home;
	io.KeyMap[ImGuiKey_End] = (int)InputKey::End;
	io.KeyMap[ImGuiKey_Insert] = (int)InputKey::Insert;
	io.KeyMap[ImGuiKey_Delete] = (int)InputKey::Delete;
	io.KeyMap[ImGuiKey_Backspace] = (int)InputKey::Backspace;
	io.KeyMap[ImGuiKey_Space] = (int)InputKey::Space;
	io.KeyMap[ImGuiKey_Enter] = (int)InputKey::Return;
	io.KeyMap[ImGuiKey_Escape] = (int)InputKey::Escape;
	io.KeyMap[ImGuiKey_A] = (int)InputKey::A;
	io.KeyMap[ImGuiKey_C] = (int)InputKey::C;
	io.KeyMap[ImGuiKey_V] = (int)InputKey::V;
	io.KeyMap[ImGuiKey_X] = (int)InputKey::X;
	io.KeyMap[ImGuiKey_Y] = (int)InputKey::Y;
	io.KeyMap[ImGuiKey_Z] = (int)InputKey::Z;

	io.SetClipboardTextFn = SetClipboardTestCallback;
	io.GetClipboardTextFn = GetClipboardTestCallback;
	io.ClipboardUserData = this;

	if (!LoadFontTextures())
	{
		return false;
	}

	std::shared_ptr<Material> material = m_material.Get();
	material->GetProperties().Set(ImGuiScale, Vector2());
	material->GetProperties().Set(ImGuiTranslation, Vector2());
	material->GetProperties().Set(ImGuiTexture, m_fontTexture);
	material->UpdateResources();

	return true;
}

bool ImguiManager::LoadFontTextures()
{
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* pixels;
	int texWidth;
	int texHeight;
	
	io.Fonts->GetTexDataAsRGBA32(&pixels, &texWidth, &texHeight);
	size_t dataSize = texWidth * texHeight * 4 * sizeof(char);

	Array<char> data(dataSize);
	memcpy(data.data(), pixels, dataSize);

	SamplerDescription samplerDescription;

	String textureName = "ImGui Font Texture";

	std::shared_ptr<TextureResourceLoader> textureLoader = m_resourceManager->GetLoader<TextureResourceLoader>();
	std::shared_ptr<Texture> texture = textureLoader->CreateTextureFromBytes(textureName, data, texWidth, texHeight, samplerDescription, false);
	m_fontTexture = m_resourceManager->CreateFromPointer<Texture>(textureName, texture);

	return true;
}

void ImguiManager::Dispose()
{
	m_graphics->WaitForDeviceIdle();

	ImGui::DestroyContext();
}

void ImguiManager::UpdateInput()
{
	ImGuiIO& io = ImGui::GetIO();

	Vector2 mousePos = m_input->GetMousePosition();
	io.MousePos = ImVec2(mousePos.x, mousePos.y);
	
	for (int i = 0; i < sizeof(io.MouseDown) / sizeof(*io.MouseDown); i++)
	{
		io.MouseDown[i] = m_input->IsKeyDown((InputKey)((int)InputKey::Mouse_0 + i));
	}

	io.MouseWheel = m_input->GetMouseWheel(false);
	io.MouseWheelH = m_input->GetMouseWheel(true);

	String input = m_input->GetInput();
	io.AddInputCharactersUTF8(input.c_str());

	for (int i = 0; i < sizeof(io.KeysDown) / sizeof(*io.KeysDown); i++)
	{
		io.KeysDown[i] = m_input->IsKeyDown((InputKey)i);
	}

	io.KeyShift = m_input->IsModifierActive(InputModifier::Shift);
	io.KeyCtrl = m_input->IsModifierActive(InputModifier::Ctrl);
	io.KeyAlt = m_input->IsModifierActive(InputModifier::Alt);
	io.KeySuper = m_input->IsModifierActive(InputModifier::GUI);

	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
	{
		switch (ImGui::GetMouseCursor())
		{
		case ImGuiMouseCursor_None:			m_input->SetMouseCursor(InputCursor::None);		break;
		case ImGuiMouseCursor_Arrow:		m_input->SetMouseCursor(InputCursor::Arrow);	break;
		case ImGuiMouseCursor_TextInput:	m_input->SetMouseCursor(InputCursor::IBeam);	break;
		case ImGuiMouseCursor_ResizeAll:	m_input->SetMouseCursor(InputCursor::SizeAll);	break;
		case ImGuiMouseCursor_ResizeNS:		m_input->SetMouseCursor(InputCursor::SizeNS);	break;
		case ImGuiMouseCursor_ResizeEW:		m_input->SetMouseCursor(InputCursor::SizeWE);	break;
		case ImGuiMouseCursor_ResizeNESW:	m_input->SetMouseCursor(InputCursor::SizeNESW);	break;
		case ImGuiMouseCursor_ResizeNWSE:	m_input->SetMouseCursor(InputCursor::SizeNWSE);	break;
		}
	}

	if (io.WantSetMousePos)
	{
		m_input->SetMousePosition(Vector2(io.MousePos.x, io.MousePos.y));
	}

	m_input->SetMouseCapture(ImGui::IsAnyMouseDown());
}

void ImguiManager::StartFrame(const FrameTime& time)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(m_window->GetWidth(), m_window->GetHeight());
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.DeltaTime = time.DeltaTime;

	m_mouseRequired = m_mouseRequiredFlagged;
	m_mouseRequiredFlagged = false;

	UpdateInput();

	ImGui::NewFrame();

	UpdateDebugMenu();
}

void ImguiManager::UpdateDebugMenu()
{
	if (m_input->WasKeyPressed(InputKey::Escape))
	{
		m_debugMenuActive = !m_debugMenuActive;
	}

	if (m_debugMenuActive)
	{
		FlagMouseControlRequired();
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				// todo: exit
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void ImguiManager::FlagMouseControlRequired()
{
	m_mouseRequiredFlagged = true;
}

bool ImguiManager::IsMouseControlRequired()
{
	return m_mouseRequired;
}

void ImguiManager::EndFrame()
{
	ImGui::Render();

	ImDrawData* drawData = ImGui::GetDrawData();
	if (drawData->TotalVtxCount == 0)
	{
		return;
	}

	struct Vertex
	{
		Vector2 position;
		Vector2 uv;
		Vector4 color;
	};

	// Upload vert/index data.
	if (m_vertexBuffer == nullptr || drawData->TotalVtxCount > m_vertexBuffer->GetCapacity())
	{
		VertexBufferBindingDescription description;
		m_material.Get()->GetVertexBufferFormat(description);

		m_vertexBuffer = m_graphics->CreateVertexBuffer("ImGui Vertex Buffer", description, drawData->TotalVtxCount);
	}
	if (m_indexBuffer == nullptr || drawData->TotalIdxCount > m_indexBuffer->GetCapacity())
	{
		m_indexBuffer = m_graphics->CreateIndexBuffer("ImGui Index Buffer", sizeof(uint16_t), drawData->TotalIdxCount);
	}

	Array<char> vertexData(drawData->TotalVtxCount * sizeof(Vertex));
	Array<char> indexData(drawData->TotalIdxCount * sizeof(uint16_t));

	char* vertexPtr = vertexData.data();
	char* indexPtr = indexData.data();

	float colorNormalizer = 1.0f / 255.0f;

	for (int cmdListIndex = 0; cmdListIndex < drawData->CmdListsCount; cmdListIndex++)
	{
		const ImDrawList* cmdList = drawData->CmdLists[cmdListIndex];
		for (int vertIndex = 0; vertIndex < cmdList->VtxBuffer.Size; vertIndex++)
		{
			const ImDrawVert& imVertex = cmdList->VtxBuffer[vertIndex];

			Vertex vertex;
			vertex.position.x = imVertex.pos.x;
			vertex.position.y = imVertex.pos.y;
			vertex.uv.x = imVertex.uv.x;
			vertex.uv.y = imVertex.uv.y;
			vertex.color.r = ((imVertex.col >> IM_COL32_R_SHIFT) & 0xFF) * colorNormalizer;
			vertex.color.g = ((imVertex.col >> IM_COL32_G_SHIFT) & 0xFF) * colorNormalizer;
			vertex.color.b = ((imVertex.col >> IM_COL32_B_SHIFT) & 0xFF) * colorNormalizer;
			vertex.color.a = ((imVertex.col >> IM_COL32_A_SHIFT) & 0xFF) * colorNormalizer;

			memcpy(vertexPtr, &vertex, sizeof(Vertex));
			vertexPtr += sizeof(Vertex);
		}

		memcpy(indexPtr, cmdList->IdxBuffer.Data, sizeof(uint16_t) * cmdList->IdxBuffer.Size);
		indexPtr += sizeof(uint16_t) * cmdList->IdxBuffer.Size;
	}

	m_vertexBuffer->Stage(vertexData.data(), 0, vertexData.size());
	m_indexBuffer->Stage(indexData.data(), 0, indexData.size());

	m_renderer->QueueRenderCommand(RenderCommandStage::PostViewsRendered, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {

		int swapWidth = m_renderer->GetSwapChainWidth();
		int swapHeight = m_renderer->GetSwapChainHeight();

		int vertexOffset = 0;
		int indexOffset = 0;

		ImVec2 displayPosition = drawData->DisplayPos;
		ImVec2 displaySize = drawData->DisplaySize;

		Vector2 scale = Vector2(
			2.0f / displaySize.x,
			2.0f / displaySize.y
		);
		Vector2 translation = Vector2(
			-1.0f - displayPosition.x * scale.x,
			-1.0f - displayPosition.y * scale.y
		);

		std::shared_ptr<Material> material = m_material.Get();

		material->GetProperties().Set(ImGuiScale, scale);
		material->GetProperties().Set(ImGuiTranslation, translation);
		material->GetProperties().Set(ImGuiTexture, m_fontTexture);
		material->UpdateResources();

		buffer->Upload(m_vertexBuffer);
		buffer->Upload(m_indexBuffer);

		buffer->BeginPass(material->GetRenderPass(), m_renderer->GetCurrentFramebuffer());
		buffer->BeginSubPass();

		buffer->SetPipeline(material->GetPipeline());
		buffer->SetViewport(0, 0, swapWidth, swapHeight);

		buffer->SetIndexBuffer(m_indexBuffer);
		buffer->SetVertexBuffer(m_vertexBuffer);
		buffer->SetResourceSets({ material->GetResourceSet() });

		for (int cmdListIndex = 0; cmdListIndex < drawData->CmdListsCount; cmdListIndex++)
		{
			const ImDrawList* cmdList = drawData->CmdLists[cmdListIndex];
			for (int cmdIndex = 0; cmdIndex < cmdList->CmdBuffer.size(); cmdIndex++)
			{
				// todo: handle texture-id (we only use fonts right now so not much of an issue).

				const ImDrawCmd* cmd = &cmdList->CmdBuffer[cmdIndex];
				if (cmd->UserCallback != nullptr)
				{
					cmd->UserCallback(cmdList, cmd);
				}
				else
				{
					buffer->SetScissor(
						(int32_t)(cmd->ClipRect.x - displayPosition.x) > 0 ? (int32_t)(cmd->ClipRect.x - displayPosition.x) : 0,
						(int32_t)(cmd->ClipRect.y - displayPosition.y) > 0 ? (int32_t)(cmd->ClipRect.y - displayPosition.y) : 0,
						(uint32_t)(cmd->ClipRect.z - cmd->ClipRect.x),
						(uint32_t)(cmd->ClipRect.w - cmd->ClipRect.y)
					);

					buffer->DrawIndexedElements(cmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				}

				indexOffset += cmd->ElemCount;
			}

			vertexOffset += cmdList->VtxBuffer.Size;
		}

		buffer->EndSubPass();
		buffer->EndPass();

	});
}