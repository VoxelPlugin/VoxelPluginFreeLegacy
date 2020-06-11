// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

enum class EVoxelOctreeLeafQuery
{
	CreateIfNull,
	ReturnIfNull
};

namespace FVoxelOctreeUtilities
{
	template<EVoxelOctreeLeafQuery Query, typename T>
	inline auto GetLeaf(T& Tree, int32 X, int32 Y, int32 Z) -> decltype(&Tree.AsLeaf())
	{
		checkVoxelSlow(Tree.IsInOctree(X, Y, Z));
		auto* Ptr = &Tree;
		while (!Ptr->IsLeaf())
		{
			if (!Ptr->AsParent().HasChildren())
			{
				if (Query == EVoxelOctreeLeafQuery::CreateIfNull)
				{
					Ptr->AsParent().CreateChildren();
				}
				else
				{
					return nullptr;
				}
			}
			Ptr = &Ptr->AsParent().GetChild(X, Y, Z);
		}
		checkVoxelSlow(Ptr->IsInOctree(X, Y, Z));
		return &Ptr->AsLeaf();
	}
	template<EVoxelOctreeLeafQuery Query, typename T>
	inline auto* GetLeaf(T& Tree, const FIntVector& P)
	{
		return GetLeaf<Query>(Tree, P.X, P.Y, P.Z);
	}

	template<typename T>
	inline T& GetBottomNode(T& Tree, int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(Tree.IsInOctree(X, Y, Z));
		auto* Ptr = &Tree;
		while (!Ptr->IsLeaf() && Ptr->AsParent().HasChildren())
		{
			Ptr = &Ptr->AsParent().GetChild(X, Y, Z);
		}
		checkVoxelSlow(Ptr->IsInOctree(X, Y, Z));
		return *Ptr;
	}
	
	template<typename T, typename TId>
	inline T& GetOctreeById(T& Tree, const TId& Id)
	{
		checkVoxelSlow(Tree.IsInOctree(Id.Position));
		checkVoxelSlow(Tree.Height >= Id.Height);
		
		auto* Ptr = &Tree;
		for (int32 Height = Tree.Height; Height > Id.Height; Height--)
		{
			check(!Ptr->IsLeaf() && Ptr->AsParent().HasChildren());
			Ptr = &Ptr->AsParent().GetChild(Id.Position);
		}
		checkVoxelSlow(Ptr->Height == Id.Height);
		checkVoxelSlow(Ptr->Position == Id.Position);
		return *Ptr;
	}

	/**
	 * It is safe to create children in Apply
	 */
	template<typename T, typename F1, typename F2>
	inline void IterateTreeByPred(T& Tree, F1 ShouldApply, F2 Apply)
	{
		if (!ShouldApply(Tree))
		{
			return;
		}
		Apply(Tree);

		if (!Tree.IsLeaf() && Tree.AsParent().HasChildren())
		{
			for (auto& Child : Tree.AsParent().GetChildren())
			{
				IterateTreeByPred(Child, ShouldApply, Apply);
			}
		}
	}
	template<typename T, typename F1>
	inline void IterateEntireTree(T& Tree, F1 Apply)
	{
		IterateTreeByPred(Tree, [](auto&) { return true; }, Apply);
	}
	template<typename T, typename F1>
	inline void IterateTreeInBounds(T& Tree, const FVoxelIntBox& Bounds, F1 Apply)
	{
		IterateTreeByPred(Tree, [&](auto& IterTree) { return IterTree.GetBounds().Intersect(Bounds); }, Apply);
	}
	// Return true if the search completed.
	// To exit, return false in Apply
	template<typename T, typename F1>
	inline bool IterateTreeInBoundsEarlyExit(T& Tree, const FVoxelIntBox& Bounds, F1 Apply)
	{
		bool bContinue = true;
		IterateTreeByPred(
			Tree, 
			[&](auto& IterTree) { return bContinue && IterTree.GetBounds().Intersect(Bounds); },
			[&](auto& Chunk) { bContinue &= Apply(Chunk); });
		return bContinue;
	}

	template<typename T, typename F1, typename F2>
	inline void IterateLeavesByPred(T& Tree, F1 ShouldApply, F2 Apply)
	{
		if (!ShouldApply(Tree))
		{
			return;
		}
		if (Tree.IsLeaf())
		{
			Apply(Tree.AsLeaf());
		}
		else if (Tree.AsParent().HasChildren())
		{
			for (auto& Child : Tree.AsParent().GetChildren())
			{
				IterateLeavesByPred(Child, ShouldApply, Apply);
			}
		}
	}
	template<typename T, typename F1>
	inline void IterateAllLeaves(T& Tree, F1 Apply)
	{
		IterateLeavesByPred(Tree, [](auto&) { return true; }, Apply);
	}
	template<typename T, typename F1>
	inline void IterateLeavesInBounds(T& Tree, const FVoxelIntBox& Bounds, F1 Apply)
	{
		IterateLeavesByPred(Tree, [&](auto& IterTree) { return IterTree.GetBounds().Intersect(Bounds); }, Apply);
	}
	// Return true if the search completed.
	// To exit, return false in Apply
	template<typename T, typename F1>
	inline bool IterateLeavesInBoundsEarlyExit(T& Tree, const FVoxelIntBox& Bounds, F1 Apply)
	{
		bool bContinue = true;
		IterateLeavesByPred(
			Tree, 
			[&](auto& IterTree) { return bContinue && IterTree.GetBounds().Intersect(Bounds); },
			[&](auto& Chunk) { bContinue &= Apply(Chunk); });
		return bContinue;
	}

	template<typename U, typename T, typename F1, typename F2, typename F3>
	inline TOptional<U> ReduceByPred(T& Tree, F1 ShouldApply, F2 Apply, F3 Reduction)
	{
		if (!ShouldApply(Tree))
		{
			return {};
		}
		if (Tree.IsLeaf() || !Tree.AsParent().HasChildren())
		{
			return U{ Apply(Tree) };
		}
		else
		{
			TOptional<U> Result;
			for (auto& Child : Tree.AsParent().GetChildren())
			{
				const auto ChildResult = ReduceByPred<U>(Child, ShouldApply, Apply, Reduction);
				if (ChildResult.IsSet())
				{
					if (Result.IsSet())
					{
						Result = Reduction(Result.GetValue(), ChildResult.GetValue());
					}
					else
					{
						Result = ChildResult;
					}
				}
			}
			return Result;
		}
	}
	template<typename U, typename T, typename F1, typename F2>
	inline TOptional<U> ReduceInBounds(T& Tree, const FVoxelIntBox& Bounds, F1 Apply, F2 Reduction)
	{
		return ReduceByPred<U>(Tree, [&](auto& IterTree) { return IterTree.GetBounds().Intersect(Bounds); }, Apply, Reduction);
	}
}