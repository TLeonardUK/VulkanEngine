#include "Pch.h"

#include "Engine/Graphics/GraphicsResourceSetPool.h"

void GraphicsResourceSetDescription::AddBinding(const String& name, int location, GraphicsBindingType type, int arrayLength)
{
	GraphicsResourceSetBinding binding;
	binding.name = name;
	binding.location = location;
	binding.type = type;
	binding.arrayLength = arrayLength;

	bindings.push_back(binding);
}

bool GraphicsResourceSetDescription::EqualTo(const GraphicsResourceSetDescription& other)
{
	if (other.bindings.size() != bindings.size())
	{
		return false;
	}

	for (int i = 0; i < bindings.size(); i++)
	{
		const GraphicsResourceSetBinding& srcBinding = bindings[i];
		const GraphicsResourceSetBinding& dstBinding = other.bindings[i];

		if (srcBinding.arrayLength != dstBinding.arrayLength)
		{
			return false;
		}
		if (srcBinding.location != dstBinding.location)
		{
			return false;
		}
		if (srcBinding.name != dstBinding.name)
		{
			return false;
		}
		if (srcBinding.type != dstBinding.type)
		{
			return false;
		}
	}

	return true;
}