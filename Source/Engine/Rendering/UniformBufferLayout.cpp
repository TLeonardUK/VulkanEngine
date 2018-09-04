#include "Pch.h"

#include "Engine/Rendering/UniformBufferLayout.h"
#include "Engine/Rendering/RenderPropertyCollection.h"

#include "Engine/Graphics/GraphicsUniformBuffer.h"

#include "Engine/Engine/Logging.h"

#include "Engine/Types/Hash.h"

int UniformBufferLayout::GetSize() const
{
	int size = 0;
	int baseAlignment = 0;

	for (auto& field : Fields)
	{
		for (int i = 0; i < field.ArrayLength; i++)
		{
			int alignment = GetAlignmentForGraphicsBindingFormat(field.Format, field.ArrayLength > 1);
			if (baseAlignment == 0)
			{
				baseAlignment = alignment;
			}

			int offset = (size % alignment);
			if (offset > 0)
			{
				size += (alignment - offset);
			}
			size += GetByteSizeForGraphicsBindingFormat(field.Format);
		}
	}

	int offset = (size % baseAlignment);
	if (offset > 0)
	{
		size += (baseAlignment - offset);
	}

	return size;
}

int UniformBufferLayout::GetFieldOffset(int fieldIndex, int elementIndex) const
{
	int size = 0;
	for (int i = 0; i < Fields.size(); i++)
	{
		auto& field = Fields[i];
		for (int j = 0; j < field.ArrayLength; j++)
		{
			int alignment = GetAlignmentForGraphicsBindingFormat(field.Format, field.ArrayLength > 1);
			
			int offset = (size % alignment);
			if (offset > 0)
			{
				size += (alignment - offset);
			}

			if (i == fieldIndex && j == elementIndex)
			{
				return size;
			}

			size += GetByteSizeForGraphicsBindingFormat(field.Format);
		}
	}
	return size;
}


void UniformBufferLayout::CalculateHashCode()
{
	HashCode = 1;
	CombineHash(HashCode, Name);
	CombineHash(HashCode, Fields.size());
	
	for (int i = 0; i < Fields.size(); i++)
	{
		CombineHash(HashCode, Fields[i].Name);
		CombineHash(HashCode, Fields[i].Format);
		CombineHash(HashCode, Fields[i].BindToHash);
	}
}

void UniformBufferLayout::FillBuffer(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphicsUniformBuffer> buffer, RenderPropertyCollection** collections, int collectionCount)
{	
	m_dataBuffer.resize(GetSize());

	for (int i = 0; i < Fields.size(); i++)
	{
		const UniformBufferLayoutField& field = Fields[i];

		RenderProperty* matBinding = nullptr;
		for (int i = 0; i < collectionCount; i++)
		{
			if (collections[i] != nullptr &&
				collections[i]->Get(field.BindToHash, &matBinding))
			{
				break;
			}
		}

		if (matBinding == nullptr)
		{
			logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', property '%s' does not exist.", Name.c_str(), field.Name.c_str(), field.BindTo.c_str());
			continue;
		}

		if (matBinding->Values.size() != field.ArrayLength)
		{
			logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', property '%s' has array length '%i' not expected '%i'.", Name.c_str(), field.Name.c_str(), field.BindTo.c_str(), matBinding->Values.size(), field.ArrayLength);
			continue;
		}

		for (int element = 0; element < field.ArrayLength; element++)
		{
			int fieldOffset = GetFieldOffset(i, element);

			// insert into ubo.
			char* destination = m_dataBuffer.data() + fieldOffset;
			char* source;
			int sourceSize;

			switch (field.Format)
			{
			case GraphicsBindingFormat::Bool:		sourceSize = sizeof(bool);		source = reinterpret_cast<char*>(&matBinding->Values[element].Bool);		break;
			case GraphicsBindingFormat::Bool2:		sourceSize = sizeof(BVector2);	source = reinterpret_cast<char*>(&matBinding->Values[element].Bool2);		break;
			case GraphicsBindingFormat::Bool3:		sourceSize = sizeof(BVector3);	source = reinterpret_cast<char*>(&matBinding->Values[element].Bool3);		break;
			case GraphicsBindingFormat::Bool4:		sourceSize = sizeof(BVector4);	source = reinterpret_cast<char*>(&matBinding->Values[element].Bool4);		break;
			case GraphicsBindingFormat::Int:		sourceSize = sizeof(int32_t);	source = reinterpret_cast<char*>(&matBinding->Values[element].Int);			break;
			case GraphicsBindingFormat::Int2:		sourceSize = sizeof(IVector2);	source = reinterpret_cast<char*>(&matBinding->Values[element].Int2);		break;
			case GraphicsBindingFormat::Int3:		sourceSize = sizeof(IVector3);	source = reinterpret_cast<char*>(&matBinding->Values[element].Int3);		break;
			case GraphicsBindingFormat::Int4:		sourceSize = sizeof(IVector4);	source = reinterpret_cast<char*>(&matBinding->Values[element].Int4);		break;
			case GraphicsBindingFormat::UInt:		sourceSize = sizeof(uint32_t);	source = reinterpret_cast<char*>(&matBinding->Values[element].UInt);		break;
			case GraphicsBindingFormat::UInt2:		sourceSize = sizeof(UVector2);	source = reinterpret_cast<char*>(&matBinding->Values[element].UInt2);		break;
			case GraphicsBindingFormat::UInt3:		sourceSize = sizeof(UVector3);	source = reinterpret_cast<char*>(&matBinding->Values[element].UInt3);		break;
			case GraphicsBindingFormat::UInt4:		sourceSize = sizeof(UVector4);	source = reinterpret_cast<char*>(&matBinding->Values[element].UInt4);		break;
			case GraphicsBindingFormat::Float:		sourceSize = sizeof(float);		source = reinterpret_cast<char*>(&matBinding->Values[element].Float);		break;
			case GraphicsBindingFormat::Float2:		sourceSize = sizeof(Vector2);	source = reinterpret_cast<char*>(&matBinding->Values[element].Float2);		break;
			case GraphicsBindingFormat::Float3:		sourceSize = sizeof(Vector3);	source = reinterpret_cast<char*>(&matBinding->Values[element].Float3);		break;
			case GraphicsBindingFormat::Float4:		sourceSize = sizeof(Vector4);	source = reinterpret_cast<char*>(&matBinding->Values[element].Float4);		break;
			case GraphicsBindingFormat::Double:		sourceSize = sizeof(double);	source = reinterpret_cast<char*>(&matBinding->Values[element].Double);		break;
			case GraphicsBindingFormat::Double2:	sourceSize = sizeof(DVector2);	source = reinterpret_cast<char*>(&matBinding->Values[element].Double2);		break;
			case GraphicsBindingFormat::Double3:	sourceSize = sizeof(DVector3);	source = reinterpret_cast<char*>(&matBinding->Values[element].Double3);		break;
			case GraphicsBindingFormat::Double4:	sourceSize = sizeof(DVector4);	source = reinterpret_cast<char*>(&matBinding->Values[element].Double4);		break;
			case GraphicsBindingFormat::Matrix2:	sourceSize = sizeof(Matrix2);	source = reinterpret_cast<char*>(&matBinding->Values[element].Matrix2);		break;
			case GraphicsBindingFormat::Matrix3:	sourceSize = sizeof(Matrix3);	source = reinterpret_cast<char*>(&matBinding->Values[element].Matrix3);		break;
			case GraphicsBindingFormat::Matrix4:	sourceSize = sizeof(Matrix4);	source = reinterpret_cast<char*>(&matBinding->Values[element].Matrix4);		break;
			default:
			{
				logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', invalid format.", Name.c_str(), field.Name.c_str());
				continue;
			}
			}

			memcpy(destination, source, sourceSize);
		}
	}

	buffer->Upload(m_dataBuffer.data(), 0, (int)m_dataBuffer.size());
}