#pragma once

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/Types/Texture.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"

typedef int MaterialPropertyHash;

MaterialPropertyHash CalculateMaterialPropertyHash(const String& name);

struct MaterialProperty
{
public:
	GraphicsBindingFormat Format;
	MaterialPropertyHash Hash;
	String Name;

	union
	{
		bool		Value_Bool;
		BVector2	Value_Bool2;
		BVector3	Value_Bool3;
		BVector4	Value_Bool4;

		int32_t		Value_Int;
		IVector2	Value_Int2;
		IVector3	Value_Int3;
		IVector4	Value_Int4;

		uint32_t	Value_UInt;
		UVector2	Value_UInt2;
		UVector3	Value_UInt3;
		UVector4	Value_UInt4;

		float		Value_Float;
		Vector2		Value_Float2;
		Vector3		Value_Float3;
		Vector4		Value_Float4;

		double		Value_Double;
		DVector2	Value_Double2;
		DVector3	Value_Double3;
		DVector4	Value_Double4;

		Matrix2		Value_Matrix2;
		Matrix3		Value_Matrix3;
		Matrix4		Value_Matrix4;
	};

	ResourcePtr<Texture> Value_Texture;

	void ParseJsonValue(Array<json>& value);

};

struct MaterialPropertyCollection
{
private:
	Dictionary<MaterialPropertyHash, MaterialProperty> m_properties;

protected:
	MaterialProperty* GetOrCreate(MaterialPropertyHash name);

public:
	void Add(const MaterialProperty& prop);

	bool Get(MaterialPropertyHash Hash, MaterialProperty** binding);

	void Set(MaterialPropertyHash name, bool value);
	void Set(MaterialPropertyHash name, BVector2 value);
	void Set(MaterialPropertyHash name, BVector3 value);
	void Set(MaterialPropertyHash name, BVector4 value);

	void Set(MaterialPropertyHash name, int32_t value);
	void Set(MaterialPropertyHash name, IVector2 value);
	void Set(MaterialPropertyHash name, IVector3 value);
	void Set(MaterialPropertyHash name, IVector4 value);

	void Set(MaterialPropertyHash name, uint32_t value);
	void Set(MaterialPropertyHash name, UVector2 value);
	void Set(MaterialPropertyHash name, UVector3 value);
	void Set(MaterialPropertyHash name, UVector4 value);

	void Set(MaterialPropertyHash name, float value);
	void Set(MaterialPropertyHash name, Vector2 value);
	void Set(MaterialPropertyHash name, Vector3 value);
	void Set(MaterialPropertyHash name, Vector4 value);

	void Set(MaterialPropertyHash name, double value);
	void Set(MaterialPropertyHash name, DVector2 value);
	void Set(MaterialPropertyHash name, DVector3 value);
	void Set(MaterialPropertyHash name, DVector4 value);

	void Set(MaterialPropertyHash name, Matrix2 value);
	void Set(MaterialPropertyHash name, Matrix3 value);
	void Set(MaterialPropertyHash name, Matrix4 value);

	void Set(MaterialPropertyHash name, ResourcePtr<Texture> value);

};