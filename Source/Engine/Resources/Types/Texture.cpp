#include "Engine/Resources/Types/Texture.h"

const char* Texture::Tag = "texture";

Texture::Texture(std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView)
	: m_image(image)
	, m_imageView(imageView)
{
}