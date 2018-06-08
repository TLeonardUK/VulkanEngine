#include "Engine/Resources/Types/Texture.h"

const char* Texture::Tag = "Texture";

Texture::Texture(std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler)
	: m_image(image)
	, m_imageView(imageView)
	, m_sampler(sampler)
{
}