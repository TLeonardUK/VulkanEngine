#pragma once

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/Types/Material.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"
#include "Engine/Utilities/Enum.h"

class TextureResourceLoader;
class Shader;
class Binding;
class IGraphicsIndexBufer;
class IGraphicsVertexBufer;

struct ModelSubMesh
{
public:
	std::shared_ptr<IGraphicsIndexBufer> m_indexBuffer;
	std::shared_ptr<IGraphicsVertexBufer> m_vertexBuffer;

};

class Model
	: public IResource
{
private:
	Array<ResourcePtr<Material>> m_defaultMaterials;
	Array<ModelSubMesh> m_subMeshes;

public:
	static const char* Tag;

	Model();

};
