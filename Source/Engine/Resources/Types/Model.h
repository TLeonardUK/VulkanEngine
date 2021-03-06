#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/Types/Material.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsVertexBuffer.h"
#include "Engine/Graphics/GraphicsIndexBuffer.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Bounds.h"
#include "Engine/Types/Dictionary.h"
#include "Engine/Utilities/Enum.h"

class TextureResourceLoader;
class Shader;
class Binding;
class IGraphics;

class Mesh
{
private:
	std::shared_ptr<Logger> m_logger;
	String m_name;

	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Renderer> m_renderer;

	Mutex m_updateResourcesMutex;

	ResourcePtr<Material> m_material;
	Array<Vector3> m_vertices;
	Array<Vector3> m_normals;
	Dictionary<int, Array<Vector2>> m_texCoords;
	Array<Vector4> m_colors;
	Array<int> m_indices;

	int m_indexCount;

	//Array<char> m_interleavedData;

	std::shared_ptr<Material> m_lastUpdatedMaterial;
	std::shared_ptr<Shader> m_lastUpdatedShader;

	VertexBufferBindingDescription m_vertexBufferFormat;

	std::shared_ptr<IGraphicsIndexBuffer> m_indexBuffer;
	std::shared_ptr<IGraphicsVertexBuffer> m_vertexBuffer;

	Bounds m_bounds;

	bool m_dirty;

	bool m_keepCpuShadowCopy;

private:
	friend class Model;
	friend class Renderer;
	friend struct MeshBatcher;

	const std::shared_ptr<IGraphicsIndexBuffer>& GetIndexBuffer();
	const std::shared_ptr<IGraphicsVertexBuffer>& GetVertexBuffer();
	int GetIndexCount();

	void UpdateResources();

public:
	Mesh(std::shared_ptr<Logger> logger, std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphics> graphics, const String& name);
	~Mesh();

	ResourcePtr<Material> GetMaterial();
	Bounds GetBounds();

	void SetMaterial(ResourcePtr<Material> material);
	void SetVertices(const Array<Vector3>& vertices);
	void SetNormals(const Array<Vector3>& normals);
	void SetTexCoords(int index, const Array<Vector2>& texCoords);
	void SetColors(const Array<Vector4>& colors);
	void SetIndices(const Array<int>& indices);

	String GetName();

	void RecalculateBounds();

};

class Model
	: public IResource
{
private:
	std::shared_ptr<Logger> m_logger;
	String m_name;

	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Renderer> m_renderer;
	Array<std::shared_ptr<Mesh>> m_meshes;

private:
	friend class Renderer;
	friend class ModelResourceLoader;

	void UpdateResources();

public:
	static const char* Tag;

	Model(std::shared_ptr<Logger> logger, std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphics> graphics, const String& Name);
	virtual ~Model();

	std::shared_ptr<Mesh> CreateMesh();

	const Array<std::shared_ptr<Mesh>>& GetMeshes();

};
