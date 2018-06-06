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
