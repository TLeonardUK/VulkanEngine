#include "Pch.h"

#include "Engine/Rendering/MaterialResourceSet.h"
#include "Engine/Rendering/RenderPropertyCollection.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/TextureCube.h"
#include "Engine/Types/Hash.h"
#include "Engine/Engine/Logging.h"

void MaterialResourceSet::CalculateHashCode()
{
	hashCode = 1;

	CombineHash(hashCode, bindings.size());

	for (ShaderBinding& binding : bindings)
	{
		CombineHash(hashCode, binding.Binding);
		CombineHash(hashCode, binding.Set);
		CombineHash(hashCode, binding.BindTo);
		CombineHash(hashCode, binding.Name);
		CombineHash(hashCode, binding.Type);
		CombineHash(hashCode, binding.UniformBufferLayout.HashCode);
	}
}

void MaterialResourceSet::UpdateBindings(
	const std::shared_ptr<Logger>& logger,
	const std::shared_ptr<IGraphics>& graphics,
	RenderPropertyCollection* collection,
	const std::shared_ptr<IGraphicsResourceSet>& updateSet
)
{
	int meshUboIndex = 0;

	for (ShaderBinding& binding : bindings)
	{
		switch (binding.Type)
		{
		case GraphicsBindingType::UniformBufferObject:
			{
				// todo: not sure why on earth you would every want a UBO array, but *shrugs* this keeps
				// everything consistent.
				for (int i = 0; i < binding.ArrayLength; i++)
				{
					std::shared_ptr<IGraphicsUniformBuffer> buffer = nullptr;

					buffer = collection->GetUniformBuffer(graphics, logger, binding.UniformBufferLayout);
					assert(buffer != nullptr);

					updateSet->UpdateBinding(binding.Binding, i, buffer);
				}
				break;
			}
		case GraphicsBindingType::Sampler:
			{
				RenderProperty* matBinding = nullptr;			
				if (!collection->Get(binding.BindToHash, &matBinding))
				{
					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property '%s' does not exist.", name.c_str(), binding.Name.c_str(), binding.BindTo.c_str());
					continue;
				}

				if (matBinding->Format != GraphicsBindingFormat::Texture)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::Texture);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property format is '%s' expected '%s'.", name.c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				if (matBinding->Values.size() != binding.ArrayLength)
				{
					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, array length is '%i' expected '%i'.", name.c_str(), binding.Name.c_str(), matBinding->Values.size(), binding.ArrayLength);
					continue;
				}

				for (int i = 0; i < binding.ArrayLength; i++)
				{
					std::shared_ptr<Texture> texture = matBinding->Values[i].Texture.Get();
					if (matBinding->Values[i].ImageSampler.ImageSampler == nullptr &&
						matBinding->Values[i].ImageSampler.ImageView == nullptr)
					{
						updateSet->UpdateBinding(binding.Binding, i, texture->GetSampler(), texture->GetImageView());
					}
					else
					{
						updateSet->UpdateBinding(binding.Binding, i, matBinding->Values[i].ImageSampler.ImageSampler, matBinding->Values[i].ImageSampler.ImageView);
					}
				}

				break;
			}
		case GraphicsBindingType::SamplerCube:
			{		
				RenderProperty* matBinding = nullptr;
				if (!collection->Get(binding.BindToHash, &matBinding))
				{
					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property '%s' does not exist.", name.c_str(), binding.Name.c_str(), binding.BindTo.c_str());
					continue;
				}

				if (matBinding->Format != GraphicsBindingFormat::TextureCube)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::TextureCube);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property format is '%s' expected '%s'.", name.c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				if (matBinding->Values.size() != binding.ArrayLength)
				{
					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, array length is '%i' expected '%i'.", name.c_str(), binding.Name.c_str(), matBinding->Values.size(), binding.ArrayLength);
					continue;
				}

				for (int i = 0; i < binding.ArrayLength; i++)
				{
					std::shared_ptr<TextureCube> texture = matBinding->Values[i].TextureCube.Get();
					if (matBinding->Values[i].ImageSampler.ImageSampler == nullptr &&
						matBinding->Values[i].ImageSampler.ImageView == nullptr)
					{
						updateSet->UpdateBinding(binding.Binding, i, texture->GetSampler(), texture->GetImageView());
					}
					else
					{
						updateSet->UpdateBinding(binding.Binding, i, matBinding->Values[i].ImageSampler.ImageSampler, matBinding->Values[i].ImageSampler.ImageView);
					}
				}

				break;
			}
		}
	}
}