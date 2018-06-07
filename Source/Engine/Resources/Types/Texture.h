#pragma once

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"

class IGraphicsImage;
class IGraphicsImageView;
class TextureResourceLoader;

class Texture
	: public IResource
{
private:
	std::shared_ptr<IGraphicsImage> m_image;
	std::shared_ptr<IGraphicsImageView> m_imageView;

public:
	static const char* Tag;

	Texture(std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView);

};
