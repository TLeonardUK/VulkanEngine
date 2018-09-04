#pragma once
#include "Pch.h"

#include "Engine/Types/Vector3.h"
#include "Engine/Types/Sphere.h"
#include "Engine/Types/Bounds.h"
#include "Engine/Types/Frustum.h"
#include "Engine/Types/Dictionary.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

template <typename ValueType>
struct OctTree
{
public:
	typedef uint64_t EntryId;

	struct Entry
	{
		EntryId id;
		Bounds bounds;
		ValueType value;
	};

	struct Node
	{
		int level;
		Bounds bounds;
		Node* children[8];
		Bounds childrenBounds[8];
		Array<Entry> entries;

		Node(int level_, const Bounds& bounds_)
			: level(level_)
			, bounds(bounds_)
		{
			Reset();
		}

		void Reset()
		{
			memset(children, 0, sizeof(children));
			bounds.GetSubDevisions(childrenBounds);
		}
	};

	struct Token
	{
	private:
		friend struct OctTree<ValueType>;

		EntryId entryId;
		Node* node;
	};

	struct Result
	{
		Array<ValueType> entries;
		Array<const Node*> nodes;
	};

private:
	typedef std::function<bool(const Bounds& bounds)> GetIntersectionFunction_t;

	int m_leafLevel;
	Node m_root;
	uint64_t m_nextEntryId;
	Array<Node*> m_nodes;

private:
	Token Insert(const Bounds& area, Entry& entry, Node& node)
	{
		int fitsInChild = -1;

		// Can we fit into any of the nodes children?
		if (node.level < m_leafLevel)
		{
			for (int i = 0; i < 8; i++)
			{
				if (node.childrenBounds[i].Contains(area))
				{
					if (fitsInChild != -1)
					{
						fitsInChild = -1;
						break;
					}
					fitsInChild = i;
				}
			}
		}

		// If we have an exclusive child we can fit in, insert into it.
		if (fitsInChild >= 0)
		{
			if (node.children[fitsInChild] == nullptr)
			{
				node.children[fitsInChild] = new Node(node.level + 1, node.childrenBounds[fitsInChild]);
				m_nodes.push_back(node.children[fitsInChild]);
			}

			return Insert(area, entry, *node.children[fitsInChild]);
		}
		// Otherwise we are the largest node that can contain area, so insert us.
		else
		{
			node.entries.push_back(entry);

			Token result;
			result.entryId = entry.id;
			result.node = &node;

			return result;
		}
	}

	void GetNodes(Result& result, const Node& node, GetIntersectionFunction_t intersectFunction) const
	{
		ProfileScope scope(ProfileColors::Cpu, "Cell");
		
		if (node.entries.size() > 0)
		{
			result.nodes.push_back(&node);
		}

		for (int i = 0; i < 8; i++)
		{
			if (node.children[i] != nullptr &&
				intersectFunction(node.childrenBounds[i]))
			{
				GetNodes(result, *node.children[i], intersectFunction);
			}
		}
	}

	void GetResultEntries(Result& result, GetIntersectionFunction_t intersectFunction, bool bCoarse) const
	{
		int maxResults = 0;

		for (auto& subNode : result.nodes)
		{
			maxResults += (int)subNode->entries.size();
		}

		result.entries.resize(maxResults);

		std::atomic<int> totalResults = 0;

		ParallelFor(static_cast<int>(result.nodes.size()), [&](int i)
		{
			auto& node = result.nodes[i];
			
			for (int i = 0; i < node->entries.size(); i++)
			{
				auto& entryPair = node->entries[i];

				if (bCoarse || intersectFunction(entryPair.bounds))
				{
					int index = totalResults++;
					result.entries[index] = entryPair.value;
				}
			}
		}, 1, "OctTree Gather");

		{
			ProfileScope scope(ProfileColors::Cpu, "Resize");

			result.entries.resize(totalResults);
		}
	}

public:
	OctTree() = default;

	OctTree(const Vector3& extents, int maxLevels)
		: m_nextEntryId(1)
		, m_leafLevel(maxLevels)
		, m_root(0, Bounds(-extents * 0.5f, extents*0.5f))
	{
		m_nodes.push_back(&m_root);
	}

	~OctTree()
	{
		Reset();
	}

	void Reset()
	{
		for (auto& node : m_nodes)
		{
			if (node != &m_root)
			{
				delete node;
			}
		}
		m_nodes.clear();
		m_root.Reset();
	}

	Token Add(const Bounds& area, ValueType value)
	{
		Entry entry;
		entry.id = m_nextEntryId++;
		entry.bounds = area;
		entry.value = value;

		return Insert(area, entry, m_root);
	}

	void Remove(Token token)
	{
		// Sloooooooooww, but preferential for linear-access when searching.

		Array<Entry>& entries = token.node->entries;
		for (int i = 0; i < entries.size(); i++)
		{
			Entry& entry = entries[i];
			if (entry.id == token.entryId)
			{
				entries[i] = entries[entries.size() - 1];
				entries.resize(entries.size() - 1);
				return;
			}
		}
	}

	void GetNodes(Array<const Node*>& nodes)
	{
		for (auto& node : m_nodes)
		{
			nodes.push_back(node);
		}
	}

	void Get(const Sphere& sphere, Result& result, bool bCoarse = false) const
	{
		GetIntersectionFunction_t func = [&sphere](const Bounds& nodeBounds) {
			return sphere.Intersects(nodeBounds);
		};
		
		result.nodes.clear();
		result.entries.clear();

		GetNodes(result, m_root, func);
		GetResultEntries(result, func, bCoarse);
	}

	void Get(const Bounds& bounds, Result& result, bool bCoarse = false) const
	{
		GetIntersectionFunction_t func = [&bounds](const Bounds& nodeBounds) {
			return bounds.Intersects(nodeBounds);
		};

		result.nodes.clear();
		result.entries.clear();

		GetNodes(result, m_root, func);
		GetResultEntries(result, func, bCoarse);
	}

	void Get(const Frustum& frustum, Result& result, bool bCoarse = false) const
	{
		GetIntersectionFunction_t func = [&frustum](const Bounds& nodeBounds) -> bool {
			return frustum.Intersects(nodeBounds) != FrustumIntersection::Outside;
		};

		result.nodes.clear();
		result.entries.clear();

		GetNodes(result, m_root, func);
		GetResultEntries(result, func, bCoarse);
	}
};
