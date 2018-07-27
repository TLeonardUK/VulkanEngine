#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/GraphicsEnums.h"

class IGraphicsResourceSet;

struct GraphicsResourceSetBinding
{
public:
	String name;
	int location;
	GraphicsBindingType type;
	int arrayLength;
};

struct GraphicsResourceSetDescription
{
public:
	Array<GraphicsResourceSetBinding> bindings;

public:
	void AddBinding(const String& name, int location, GraphicsBindingType type, int arrayLength = 1);

	bool EqualTo(const GraphicsResourceSetDescription& other);

};

class IGraphicsResourceSetPool
{
protected:
	IGraphicsResourceSetPool() { };

public:
	virtual ~IGraphicsResourceSetPool() { };

	virtual std::shared_ptr<IGraphicsResourceSet> Allocate(const GraphicsResourceSetDescription& description) = 0;

};
