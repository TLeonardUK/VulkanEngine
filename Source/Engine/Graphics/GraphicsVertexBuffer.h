#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include <memory>

struct VertexBufferBindingAttribute
{
public:
	String name;
	int location;
	GraphicsBindingFormat format;
	int stride;
	int offset;
};

struct VertexBufferBindingDescription
{
public:
	Array<VertexBufferBindingAttribute> attributes;
	int vertexSize;

public:
	void AddAttribute(const String& name, int location, GraphicsBindingFormat format, int stride, int offset);
	void SetVertexSize(int size);

};

class IGraphicsVertexBuffer
{
protected:
	IGraphicsVertexBuffer() { };

public:
	virtual ~IGraphicsVertexBuffer() { };

	virtual bool Stage(void* buffer, int offset, int length) = 0;

	virtual int GetCapacity() = 0;

};
