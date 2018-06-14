#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/File.h"
#include "Engine/Utilities/Json.h"

// todo: this should use parallel io to stream in, rather than just block loading on a couple of threads.
// todo: what do if nested loads?

ResourceManager::ResourceManager(std::shared_ptr<Logger> logger)
	: m_logger(logger)
{
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::Init()
{
	m_active = true;
	m_pendingLoads = 0;
	m_ownerThread = std::this_thread::get_id();

	for (int i = 0; i < MaxResourceLoaders; i++)
	{
		m_workers.push_back(std::thread(&ResourceManager::WorkerLoop, this));
	}

	return true;
}

void ResourceManager::Dispose()
{
	m_active = false;

	for (std::thread& thread : m_workers)
	{
		m_resourceLoadPendingSemaphore.Signal();
	}

	for (std::thread& thread : m_workers)
	{
		thread.join();
	}
}

bool ResourceManager::Mount(const String& directory)
{
	// todo: could do some error checking here, check dir exist?

	m_mountedDirectories.push_back(directory);

	m_logger->WriteInfo(LogCategory::Resources, "Mounted resource directory: %s", directory.c_str());

	return true;
}

void ResourceManager::AddLoader(std::shared_ptr<IResourceLoader> loader)
{
	m_loaders.push_back(loader);

	m_logger->WriteInfo(LogCategory::Resources, "Added resource loader for tag: %s", loader->GetTag().c_str());
}

void ResourceManager::LoadDefaults()
{
	for (auto& loader : m_loaders)
	{
		loader->LoadDefaults(shared_from_this());
	}
	WaitUntilIdle();
}

bool ResourceManager::IsIdle()
{
	return m_pendingLoads == 0;
}

void ResourceManager::WaitUntilIdle()
{
	std::unique_lock<std::mutex> lock(m_idleWaitLock);

	while (!IsIdle())
	{
		m_idleWaitCondVariable.wait(lock);

		// If waiting on owner thread we need to pump the load queue.
		if (m_ownerThread == std::this_thread::get_id())
		{
			ProcessPendingLoads();
		}
	}
}

void ResourceManager::ProcessPendingLoads()
{
	std::lock_guard<std::recursive_mutex> guard(m_pendingLoadedFlagMutex);

	int resourcesLoaded = 0;
	do
	{
		resourcesLoaded = 0;

		for (auto iter = m_pendingLoadedResources.begin(); iter != m_pendingLoadedResources.end(); )
		{
			std::shared_ptr<ResourceStatus> status = *iter;
			bool bDependenciesLoaded = true;

			for (auto dependency : status->Dependencies)
			{
				if (dependency->Status != ResourceLoadStatus::Loaded)
				{
					bDependenciesLoaded = false;
					break;
				}
			}

			if (bDependenciesLoaded)
			{
				iter = m_pendingLoadedResources.erase(iter);

				status->Status = ResourceLoadStatus::Loaded;
				m_pendingLoads--;
				resourcesLoaded++;

				m_logger->WriteSuccess(LogCategory::Resources, "[%-30s] Successfully loaded", status->Path.c_str());
			}
			else
			{
				iter++;
			}
		}

	} while (resourcesLoaded > 0);
}

void ResourceManager::CollectGarbage()
{
	std::lock_guard<std::recursive_mutex> guard(m_resourcesMutex);

	Array<String> garbage;

	for (auto& pair : m_resources)
	{
		if (pair.second.use_count() == 1 &&
			pair.second->Status == ResourceLoadStatus::Loaded)
		{
			garbage.push_back(pair.first);
		}
	}

	for (String& item : garbage)
	{
		m_resources.erase(item);

		m_logger->WriteInfo(LogCategory::Resources, "[%-30s] Unloading, no longer referenced.", item.c_str());
	}
}

ResourcePtr<IResource> ResourceManager::GetResource(const String& path)
{
	std::lock_guard<std::recursive_mutex> guard(m_resourcesMutex);

	if (m_resources.count(path) > 0)
	{
		return m_resources[path];
	}
	
	return ResourcePtr<IResource>(nullptr);
}

ResourcePtr<IResource> ResourceManager::LoadTypeLess(const String& path, const String& tag)
{
	std::lock_guard<std::recursive_mutex> guard1(m_resourcesMutex);
	std::lock_guard<std::recursive_mutex> guard2(m_pendingResourcesMutex);

	std::shared_ptr<IResourceLoader> loader = GetLoaderForTag(tag);

	ResourcePtr<IResource> existingResource = GetResource(path);
	if (existingResource.m_loadState != nullptr)
	{
		if (existingResource.m_loadState->Tag.empty() && !tag.empty())
		{
			existingResource.m_loadState->Tag = tag;
			if (loader != nullptr)
			{
				loader->AssignDefault(existingResource.m_loadState);
			}
		}

		return existingResource;
	}

	m_logger->WriteInfo(LogCategory::Resources, "[%-30s] Queueing resource for load", path.c_str());

	std::shared_ptr<ResourceStatus> status = std::make_shared<ResourceStatus>();
	status->Path = path;
	status->Status = ResourceLoadStatus::Pending;
	status->ResourceManager = shared_from_this();

	if (loader != nullptr)
	{
		loader->AssignDefault(status);
	}

	m_resources.emplace(path, status);

	m_pendingLoads++;
	m_pendingResources.emplace(status);
	m_resourceLoadPendingSemaphore.Signal();

	return status;
}

ResourcePtr<IResource> ResourceManager::CreateFromTypelessPointer(const String& name, std::shared_ptr<IResource> resource, const String& tag)
{
	std::lock_guard<std::recursive_mutex> guard1(m_resourcesMutex);

	ResourcePtr<IResource> existingResource = GetResource(name);
	if (existingResource.m_loadState != nullptr)
	{
		assert(false);
	}

	std::shared_ptr<ResourceStatus> status = std::make_shared<ResourceStatus>();
	status->Path = name;
	status->Status = ResourceLoadStatus::Loaded;
	status->ResourceManager = shared_from_this();
	status->Resource = resource;

	m_resources.emplace(name, status);

	return status;
}

bool ResourceManager::ReadResourceBytes(const String& path, Array<char>& bytes)
{
	for (String& mount : m_mountedDirectories)
	{
		if (File::ReadAllBytes(StringFormat("%s/%s", mount.c_str(), path.c_str()), bytes))
		{
			return true;
		}
	}

	return false;
}

void ResourceManager::LoadResource(std::shared_ptr<ResourceStatus> resource)
{
	m_logger->WriteInfo(LogCategory::Resources, "[%-30s] Starting to load", resource->Path.c_str());

	resource->Status = ResourceLoadStatus::Loading;

	Array<char> bytes;
	if (!ReadResourceBytes(resource->Path, bytes))
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to read path", resource->Path.c_str());
		resource->Status = ResourceLoadStatus::Failed;
		return;
	}

	json jsonValue;

	try
	{
		jsonValue = json::parse(bytes);
	}
	catch (json::exception ex)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to parse json: %s", resource->Path.c_str(), ex.what());
		resource->Status = ResourceLoadStatus::Failed;
		return;
	}

	// Check tag.
	if (jsonValue.count("Type") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Resource definition does not include required paramter 'Type'", resource->Path.c_str());
		resource->Status = ResourceLoadStatus::Failed;
		return;
	}

	// Try and find a loader for the type tag.
	String type = jsonValue["Type"];
	std::shared_ptr<IResourceLoader> loader = GetLoaderForTag(type);
	if (loader == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] No loader available for resource type '%s'", resource->Path.c_str(), type.c_str());
		resource->Status = ResourceLoadStatus::Failed;
		return;
	}

	// Load the resource!
	std::shared_ptr<IResource> result = loader->Load(shared_from_this(), resource, jsonValue);
	if (result == nullptr)
	{
		resource->Status = ResourceLoadStatus::Failed;
		return;
	}

	resource->Resource = result;

	// Defer load till end of frame to ensure we don't swap a resource out right in the middle of it being used.
	{
		std::lock_guard<std::recursive_mutex> guard(m_pendingLoadedFlagMutex);
		m_pendingLoadedResources.push_back(resource);
	}
}

std::shared_ptr<ResourceStatus> ResourceManager::GetPendingResource()
{
	std::lock_guard<std::recursive_mutex> guard(m_pendingResourcesMutex);

	if (m_pendingResources.size() > 0)
	{
		std::shared_ptr<ResourceStatus> result = m_pendingResources.front();
		m_pendingResources.pop();
		return result;
	}

	return nullptr;
}

void ResourceManager::WorkerLoop()
{
	while (m_active)
	{
		m_resourceLoadPendingSemaphore.Wait();

		std::shared_ptr<ResourceStatus> status = GetPendingResource();
		if (status != nullptr)
		{
			LoadResource(status);

			// Wake everyone up whos waiting for idle states.
			{
				std::unique_lock<std::mutex> lock(m_idleWaitLock);
				m_idleWaitCondVariable.notify_all();
			}
		}
	}
}

ResourcePtr<IResource> ResourceManager::GetDefaultForTag(const String& tag)
{
	std::shared_ptr<ResourceStatus> status = std::make_shared<ResourceStatus>();
	status->Path = "Internal:Default " + tag;
	status->Status = ResourceLoadStatus::Loaded;
	status->ResourceManager = shared_from_this();

	std::shared_ptr<IResourceLoader> loader = GetLoaderForTag(tag);
	if (loader != nullptr)
	{
		loader->AssignDefault(status);
	}

	return status;
}

std::shared_ptr<IResourceLoader> ResourceManager::GetLoaderForTag(const String& tag)
{
	for (auto& loader : m_loaders)
	{
		if (loader->GetTag() == tag)
		{
			return loader;
		}
	}

	return nullptr;
}

void ResourceStatus::WaitUntilLoaded()
{
	std::unique_lock<std::mutex> lock(ResourceManager->m_idleWaitLock);

	while (Status == ResourceLoadStatus::Loading ||
		   Status == ResourceLoadStatus::Pending)
	{
		ResourceManager->m_idleWaitCondVariable.wait(lock);

		// If waiting on owner thread we need to pump the load queue.
		if (ResourceManager->m_ownerThread == std::this_thread::get_id())
		{
			ResourceManager->ProcessPendingLoads();
		}
	}
}
