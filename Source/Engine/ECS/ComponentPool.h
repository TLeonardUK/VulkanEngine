#pragma once
#include "Pch.h"

#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Array.h"

class Logger;
class World;

class ComponentPoolBase
{
public:
	virtual ~ComponentPoolBase()
	{
	}

	virtual void* GetIndexPtr(uint64_t index) = 0;
	virtual uint64_t AllocateIndex() = 0;
	virtual void FreeIndex(uint64_t index) = 0;

};

template<typename ComponentType>
class ComponentPool : public ComponentPoolBase
{
public:
	struct ComponentState
	{
		char buffer[sizeof(ComponentType)];
	};

private:	
	struct Block
	{
	public:
		static const int BlockSize = 1000;

		Array<ComponentState> components;
		Array<uint64_t> freeIndices;

		Block(uint64_t blockBaseIndex)
		{
			for (int i = 0; i < BlockSize; i++)
			{
				components.push_back(ComponentState());
				freeIndices.push_back(blockBaseIndex + (BlockSize - (i + 1))); // Reverse index so we allocate in increasing memory address.
			}
		}
	};

	Array<Block> m_blocks;
	uint64_t m_baseIndex;

public:
	ComponentPool(uint64_t baseIndex)
		: m_baseIndex(baseIndex)
	{
	}

	virtual uint64_t AllocateIndex() override
	{
		for (Block& block : m_blocks)
		{
			if (block.freeIndices.size() > 0)
			{
				uint64_t index = block.freeIndices.back();
				block.freeIndices.pop_back();

				ComponentType* data = &GetIndex(index);
				memset(data, 0xFC, sizeof(ComponentType));
				new(data) ComponentType();

				return index;
			}
		}

		m_blocks.push_back(Block(m_baseIndex + (m_blocks.size() * Block::BlockSize)));
		uint64_t index = AllocateIndex();

		ComponentType* data = &GetIndex(index);
#if DEBUG_BUILD
		memset(data, 0xFE, sizeof(ComponentType));
#endif
		new(data) ComponentType();

		return index;
	}

	virtual void FreeIndex(uint64_t index) override
	{
		ComponentType* data = &GetIndex(index);
		data->~ComponentType();
#if DEBUG_BUILD
		memset(data, 0xAB, sizeof(ComponentType));
#endif

		uint64_t poolRelativeIndex = (index - m_baseIndex);
		uint64_t blockIndex = poolRelativeIndex / Block::BlockSize;
		m_blocks[blockIndex].freeIndices.push_back(index);
	}

	virtual void* GetIndexPtr(uint64_t index) override
	{
		uint64_t poolRelativeIndex = (index - m_baseIndex);
		uint64_t blockIndex = poolRelativeIndex / Block::BlockSize;
		Block& block = m_blocks[blockIndex];
		return &block.components[poolRelativeIndex - (blockIndex * Block::BlockSize)].buffer;
	}

	ComponentType& GetIndex(uint64_t index)
	{
		uint64_t poolRelativeIndex = (index - m_baseIndex);
		uint64_t blockIndex = poolRelativeIndex  / Block::BlockSize;
		Block& block = m_blocks[blockIndex];
		return *reinterpret_cast<ComponentType*>(&block.components[poolRelativeIndex - (blockIndex * Block::BlockSize)].buffer);
	}
};

