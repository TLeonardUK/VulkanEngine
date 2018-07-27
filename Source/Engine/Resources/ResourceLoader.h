#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Utilities/Json.h"

class IResource;
struct ResourceStatus;
class ResourceManager;

class IResourceLoader
{
private:

public:
	virtual String GetTag() = 0;

	virtual void LoadDefaults(std::shared_ptr<ResourceManager> manager) = 0;

	virtual void AssignDefault(std::shared_ptr<ResourceStatus> resource) = 0;
	
	virtual std::shared_ptr<IResource> Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue) = 0;

};
