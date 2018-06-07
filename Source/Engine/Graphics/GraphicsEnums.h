#pragma once

enum class GraphicsFormat
{
	UNORM_R8,
	UNORM_R8G8B8,
	UNORM_R8G8B8A8,
	UNORM_B8G8R8,
	UNORM_B8G8R8A8,
	UNORM_R16G16B16,
	UNORM_R16G16B16A16,
	UNORM_D24_UINT_S8,
	Undefined
};

enum class GraphicsBindingFormat
{
	Bool1,
	Bool2,
	Bool3,
	Bool4,
	Int1,
	Int2,
	Int3,
	Int4,
	UInt1,
	UInt2,
	UInt3,
	UInt4,
	Float,
	Float2,
	Float3,
	Float4,
	Double1,
	Double2,
	Double3,
	Double4,
};

enum class GraphicsBindingType
{
	UniformBufferObject,
	Sampler
};

enum class GraphicsAddressMode
{
	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder
};

enum class GraphicsFilter
{
	Linear,
	NearestNeighbour
};

enum class GraphicsMipMapMode
{
	Linear,
	NearestNeighbour
};