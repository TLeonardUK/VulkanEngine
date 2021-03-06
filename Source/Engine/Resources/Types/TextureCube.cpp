#include "Pch.h"

#include "Engine/Resources/Types/TextureCube.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"
#include "Engine/Utilities/Statistic.h"

const char* TextureCube::Tag = "TextureCube";

Statistic Stat_Resources_TextureCubeCount("Resources/Texture Cube Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

TextureCube::TextureCube(std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler)
	: m_image(image)
	, m_imageView(imageView)
	, m_sampler(sampler)
	, m_renderer(renderer)
	, m_dirty(true)
{
	Stat_Resources_TextureCubeCount.Add(1);
}

TextureCube::~TextureCube()
{
	Stat_Resources_TextureCubeCount.Add(-1);
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

	m_renderer->QueueRenderCommand(RenderCommandStage::Global_PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
		buffer->Upload(m_image);
	});
}