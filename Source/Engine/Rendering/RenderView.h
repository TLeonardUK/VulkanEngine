#pragma once
#include "Pch.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Frustum.h"
#include "Engine/Types/Color.h"

struct RenderMesh
{
	std::shared_ptr<Mesh> Mesh;
	Matrix4 Matrix;
};

struct RenderLine
{
	Vector3 Start;
	Vector3 End;
	Color	LineColor;

	RenderLine(const Vector3& start, const Vector3& end, const Color& color)
		: Start(start)
		, End(end)
		, LineColor(color)
	{
	}
};

class RenderView
{
public:
	RenderView() { }

	Matrix4 ViewMatrix;
	Matrix4 ProjectionMatrix;
	Vector3 CameraPosition;
	Rect	Viewport;
	Frustum	Frustum;

	Array<RenderMesh> Meshes;
	Array<RenderLine> Lines;

private:
	friend class Renderer;

	std::shared_ptr<IGraphicsVertexBuffer> LineVertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> LineIndexBuffer;

};