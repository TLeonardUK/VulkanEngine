#include "Pch.h"

#include "Engine/Graphics/GraphicsVertexBuffer.h"

void VertexBufferBindingDescription::AddAttribute(const String& name, int location, GraphicsBindingFormat format, int stride, int offset)
{
	VertexBufferBindingAttribute attribute;
	attribute.name = name;
	attribute.location = location;
	attribute.format = format;
	attribute.stride = stride;
	attribute.offset = offset;

	attributes.push_back(attribute);
}

void VertexBufferBindingDescription::SetVertexSize(int size)
{
	vertexSize = size;
}