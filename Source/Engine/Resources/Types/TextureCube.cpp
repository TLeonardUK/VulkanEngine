#include "Pch.h"

#include "Engine/Resources/Types/TextureCube.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"

const char* TextureCube::Tag = "TextureCube";

TextureCube::TextureCube(std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler)
	: m_image(image)
	, m_imageView(imageView)
	, m_sampler(sampler)
	, m_renderer(renderer)
	, m_dirty(true)
{
}

std::shared_ptr<IGraphicsSampler> TextureCube::GetSampler()
{
	return m_sampler;
}

std::shared_ptr<IGraphicsImageView> TextureCube::GetImageView()
{
	return m_imageView;
}

void TextureCube::UpdateResources()
{
	if (!m_dirty)
	{
		return;
	}
	m_dirty = false;

	m_renderer->QueueRenderCommand(RenderCommandStage::PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
		buffer->Upload(m_image);
	});
}