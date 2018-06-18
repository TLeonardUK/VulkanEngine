#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Queue.h"
#include "Engine/Threading/Semaphore.h"

#include <memory>
#include <thread>
#include <mutex>
#include <cassert>
#include <atomic>

class Logger;
class IResourceLoader;
class IResource;
class ResourceManager;

enum class ResourceLoadStatus
{
	NotLoaded,
	Pending,
	Loading,
	Loaded,
	Failed,
	WrongType
};

struct ResourceStatus
{
public:
	String Path;
	ResourceLoadStatus Status;
	std::shared_ptr<IResource> Resource;
	std::shared_ptr<IResource> DefaultResource;
	std::shared_ptr<ResourceManager> ResourceManager;
	Array<std::shared_ptr<ResourceStatus>> Dependencies;
	std::string Tag;

public:
	void WaitUntilLoaded();

};

template <typename ResourceType>
struct ResourcePtr
{
private:
	friend class ResourceManager;

	std::shared_ptr<ResourceStatus> m_loadState;

public:
	ResourcePtr(std::shared_ptr<ResourceStatus> status)
		: m_loadState(status)
	{
	}

	ResourcePtr()
		: m_loadState(nullptr)
	{
	}

	ResourceLoadStatus GetStatus()
	{
		if (m_loadState == nullptr)
		{
			return ResourceLoadStatus::NotLoaded;
		}

		if (m_loadState->Status == ResourceLoadStatus::Loaded)
		{
			std::shared_ptr<ResourceType> type = std::dynamic_pointer_cast<ResourceType>(m_loadState->Resource);
			if (type == nullptr)
			{
				return ResourceLoadStatus::WrongType;
			}
			return true;
		}

		return m_loadState->Status;
	}

	std::shared_ptr<ResourceType> Get()
	{
		if (m_loadState == nullptr)
		{
			return nullptr;
		}

		if (m_loadState->Status == ResourceLoadStatus::Loaded)
		{
			std::shared_ptr<ResourceType> type = std::dynamic_pointer_cast<ResourceType>(m_loadState->Resource);
			if (type != nullptr)
			{
				return type;
			}
		}

		if (m_loadState->DefaultResource != nullptr)
		{
			std::shared_ptr<ResourceType> castedDefault = std::dynamic_pointer_cast<ResourceType>(m_loadState->DefaultResource);
			if (castedDefault != nullptr)
			{
				return castedDefault;
			}
		}

		return nullptr;
	}

	void WaitUntilLoaded()
	{
		assert(m_loadState != nullptr);

		m_loadState->WaitUntilLoaded();
	}
};

class ResourceManager
	: public std::enable_shared_from_this<ResourceManager>
{
private:
	std::shared_ptr<Logger> m_logger;
	
	Array<String> m_mountedDirectories;
	Array<std::shared_ptr<IResourceLoader>> m_loaders;

	std::recursive_mutex m_resourcesMutex;
	Dictionary<String, std::shared_ptr<ResourceStatus>> m_resources;

	std::recursive_mutex m_pendingResourcesMutex;
	Queue<std::shared_ptr<ResourceStatus>> m_pendingResources;

	std::recursive_mutex m_pendingLoadedFlagMutex;
	Array<std::shared_ptr<ResourceStatus>> m_pendingLoadedResources;

	std::atomic<int> m_pendingLoads;

	//const int MaxResourceLoaders = 2;

	bool m_active;
	Array<std::thread> m_workers;

	Semaphore m_resourceLoadPendingSemaphore;

	std::mutex m_idleWaitLock;
	std::condition_variable m_idleWaitCondVariable;

	std::thread::id m_ownerThread;

private:
	friend struct ResourceStatus;

	void LoadResource(std::shared_ptr<ResourceStatus> resource);

	std::shared_ptr<ResourceStatus> GetPendingResource();

	std::shared_ptr<IResourceLoader> GetLoaderForTag(const String& tag);
	ResourcePtr<IResource> GetDefaultForTag(const String& tag);

	ResourcePtr<IResource> LoadTypeLess(const String& path, const String& tag = "");
	ResourcePtr<IResource> CreateFromTypelessPointer(const String& name, std::shared_ptr<IResource> resource, const String& tag = "");

	void WorkerLoop();

public:
	ResourceManager(std::shared_ptr<Logger> logger);
	~ResourceManager();

	bool Init();
	void Dispose();

	bool Mount(const String& directory);

	void AddLoader(std::shared_ptr<IResourceLoader> loader);

	template <typename ResourceType>
	void AddResourceDependency(std::shared_ptr<ResourceStatus> resource, ResourcePtr<ResourceType> dependency)
	{
		resource->Dependencies.push_back(dependency.m_loadState);
	}

	bool IsIdle();
	void LoadDefaults();
	void WaitUntilIdle();

	void CollectGarbage();
	void ProcessPendingLoads();

	bool ReadResourceBytes(const String& path, Array<char>& buffer);

	ResourcePtr<IResource> GetResource(const String& path);

	template <typename LoaderType>
	std::shared_ptr<LoaderType> GetLoader()
	{
		for (auto& loader : m_loaders)
		{
			std::shared_ptr<LoaderType> cast = std::dynamic_pointer_cast<LoaderType>(loader);
			if (cast != nullptr)
			{
				return cast;
			}
		}
		return nullptr;
	}

	template <typename ResourceType>
	ResourcePtr<ResourceType> GetDefault()
	{
		ResourcePtr<IResource> typeless = GetDefaultForTag(ResourceType::Tag);

		ResourcePtr<ResourceType> resource(typeless.m_loadState);

		return resource;
	}

	template <typename ResourceType>
	ResourcePtr<ResourceType> Load(const String& path)
	{
		ResourcePtr<IResource> typeless = LoadTypeLess(path, ResourceType::Tag);

		ResourcePtr<ResourceType> resource(typeless.m_loadState);

		return resource;
	}

	template <typename ResourceType>
	ResourcePtr<ResourceType> CreateFromPointer(const String& name, std::shared_ptr<ResourceType> res)
	{
		ResourcePtr<IResource> typeless = CreateFromTypelessPointer(name, res, ResourceType::Tag);

		ResourcePtr<ResourceType> resource(typeless.m_loadState);

		return resource;
	}
};
