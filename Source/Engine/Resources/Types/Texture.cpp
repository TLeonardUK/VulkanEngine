#include "Pch.h"

#include "Engine/Resources/Types/Texture.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"
#include "Engine/Utilities/Statistic.h"

const char* Texture::Tag = "Texture";

Statistic Stat_Resources_TextureCount("Resources/Texture Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

Texture::Texture(std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler)
	: m_image(image)
	, m_imageView(imageView)
	, m_sampler(sampler)
	, m_renderer(renderer)
	, m_dirty(true)
{
	Stat_Resources_TextureCount.Add(1);
}

Texture::~Texture()
{
	Stat_Resources_TextureCount.Add(-1);
}

std::shared_ptr<IGraphicsSampler> Texture::GetSampler()
{
	return m_sampler;
}

std::shared_ptr<IGraphicsImageView> Texture::GetImageView()
{
	return m_imageView;
}

void Texture::UpdateResources()
{
	if (!m_dirty)
	{
		return;
	}
	m_dirty = false;

	m_renderer->QueueRenderCommand(RenderCommandStage::Global_PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
		buffer->Upload(m_image);
	});
}