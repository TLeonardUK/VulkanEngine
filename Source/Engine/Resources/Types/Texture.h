#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"

class IGraphicsImage;
class IGraphicsImageView;
class IGraphicsSampler;
class TextureResourceLoader;
class Renderer;

class Texture
	: public IResource
{
private:
	std::shared_ptr<IGraphicsImage> m_image;
	std::shared_ptr<IGraphicsImageView> m_imageView;
	std::shared_ptr<IGraphicsSampler> m_sampler;
	std::shared_ptr<Renderer> m_renderer;

	bool m_dirty;

private:
	friend class Material;
	friend class TextureResourceLoader;

	void UpdateResources();

public:
	static const char* Tag;

	Texture(std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphicsImage> image, std::shared_ptr<IGraphicsImageView> imageView, std::shared_ptr<IGraphicsSampler> sampler);

	std::shared_ptr<IGraphicsSampler> GetSampler();
	std::shared_ptr<IGraphicsImageView> GetImageView();

};
