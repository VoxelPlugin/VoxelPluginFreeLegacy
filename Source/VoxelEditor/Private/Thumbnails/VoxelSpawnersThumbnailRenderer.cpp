// Copyright 2020 Phyronnaz

#include "VoxelSpawnersThumbnailRenderer.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelAssetSpawner.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"

#include "Engine/StaticMesh.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

bool UVoxelMeshSpawnerThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object->IsA(UVoxelMeshSpawner::StaticClass()) && CastChecked<UVoxelMeshSpawner>(Object)->Mesh;
}

void UVoxelMeshSpawnerThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily))
{
	UStaticMeshThumbnailRenderer::Draw(CastChecked<UVoxelMeshSpawner>(Object)->Mesh, X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
}

///////////////////////////////////////////////////////////////////////////////

bool UVoxelAssetSpawnerThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object->IsA(UVoxelAssetSpawner::StaticClass()) && CastChecked<UVoxelAssetSpawner>(Object)->Generator.IsValid();
}

void UVoxelAssetSpawnerThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily))
{
	const auto& Generator = CastChecked<UVoxelAssetSpawner>(Object)->Generator;
	auto* GeneratorObject = Generator.GetObject();
	if (GeneratorObject)
	{
		FThumbnailRenderingInfo* RenderInfo = GUnrealEd->GetThumbnailManager()->GetRenderingInfo(GeneratorObject);
		if (RenderInfo && RenderInfo->Renderer)
		{
			RenderInfo->Renderer->Draw(GeneratorObject, X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

auto& GetStaticStack()
{
	static TArray<UObject*> Stack;
	return Stack;
}

template<typename T>
inline void DrawGroup(UObject* Object, const int32 Num, const int32 X, const int32 Y, const uint32 Width, const uint32 Height, T Lambda)
{
	if (Width < 4 || Height < 4)
	{
		return;
	}

	if (GetStaticStack().Contains(Object))
	{
		return;
	}
	GetStaticStack().Add(Object);

	if (Num <= 4)
	{
		const uint32 HalfWidth = Width / 2;
		const uint32 HalfHeight = Height / 2;
		const int32 MiddleX = X + HalfWidth;
		const int32 MiddleY = Y + HalfHeight;
		const uint32 SecondHalfWidth = Width - HalfWidth;
		const uint32 SecondHalfHeight = Height - HalfHeight;

		const TArray<int32, TFixedAllocator<4>> Xs = {X, MiddleX, X, MiddleX};
		const TArray<int32, TFixedAllocator<4>> Ys = {Y, Y, MiddleY, MiddleY};
		const TArray<uint32, TFixedAllocator<4>> Widths = {HalfWidth, SecondHalfWidth, HalfWidth, SecondHalfWidth};
		const TArray<uint32, TFixedAllocator<4>> Heights = {HalfHeight, SecondHalfHeight, HalfHeight, SecondHalfHeight};

		for (int32 Index = 0; Index < 4; Index++)
		{
			Lambda(Index, Xs[Index], Ys[Index], Widths[Index], Heights[Index]);
		}
	}
	else
	{
		const uint32 ThirdWidth = Width / 3;
		const uint32 ThirdHeight = Height / 3;
		const int32 OneThirdX = X + ThirdWidth;
		const int32 TwoThirdX = X + 2 * ThirdWidth;
		const int32 OneThirdY = Y + ThirdHeight;
		const int32 TwoThirdY = Y + 2 * ThirdHeight;
		const uint32 LastThirdWidth = Width - 2 * ThirdWidth;
		const uint32 LastThirdHeight = Height - 2 * ThirdHeight;

		const TArray<int32, TFixedAllocator<9>> Xs = {
			X, OneThirdX, TwoThirdX,
			X, OneThirdX, TwoThirdX,
			X, OneThirdX, TwoThirdX };
		const TArray<int32, TFixedAllocator<9>> Ys = {
			Y, Y, Y,
			OneThirdY, OneThirdY, OneThirdY,
			TwoThirdY, TwoThirdY, TwoThirdY };
		const TArray<uint32, TFixedAllocator<9>> Widths = {
			ThirdWidth, ThirdWidth, LastThirdWidth,
			ThirdWidth, ThirdWidth, LastThirdWidth,
			ThirdWidth, ThirdWidth, LastThirdWidth };
		const TArray<uint32, TFixedAllocator<9>> Heights = {
			ThirdHeight,ThirdHeight,ThirdHeight,
			ThirdHeight,ThirdHeight,ThirdHeight,
			LastThirdHeight,LastThirdHeight,LastThirdHeight
		};

		for (int32 Index = 0; Index < 9; Index++)
		{
			Lambda(Index, Xs[Index], Ys[Index], Widths[Index], Heights[Index]);
		}
	}

	ensure(GetStaticStack().Pop() == Object);
}

///////////////////////////////////////////////////////////////////////////////

bool UVoxelSpawnerGroupThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object->IsA(UVoxelSpawnerGroup::StaticClass()) && CastChecked<UVoxelSpawnerGroup>(Object)->Children.Num() > 0;
}

void UVoxelSpawnerGroupThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily))
{
	auto* Group = CastChecked<UVoxelSpawnerGroup>(Object);
	auto& Children = Group->Children;
	DrawGroup(Object, Children.Num(), X, Y, Width, Height, [&](int32 Index, int32 NewX, int32 NewY, uint32 NewWidth, uint32 NewHeight)
	{
		if (Children.IsValidIndex(Index))
		{
			auto* Spawner = Children[Index].Spawner;
			if (Spawner)
			{
				FThumbnailRenderingInfo* RenderInfo = GUnrealEd->GetThumbnailManager()->GetRenderingInfo(Spawner);
				if (RenderInfo && RenderInfo->Renderer)
				{
					// TODO proper bAdditionalViewFamily
					RenderInfo->Renderer->Draw(Spawner, NewX, NewY, NewWidth, NewHeight, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
				}
			}
		}
	});
}

bool UVoxelMeshSpawnerGroupThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object->IsA(UVoxelMeshSpawnerGroup::StaticClass()) && CastChecked<UVoxelMeshSpawnerGroup>(Object)->Meshes.Num() > 0;
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelMeshSpawnerGroupThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily))
{
	auto* Group = CastChecked<UVoxelMeshSpawnerGroup>(Object);
	auto& Meshes = Group->Meshes;
	DrawGroup(Object, Meshes.Num(), X, Y, Width, Height, [&](int32 Index, int32 NewX, int32 NewY, uint32 NewWidth, uint32 NewHeight)
	{
		if (Meshes.IsValidIndex(Index))
		{
			auto* Mesh = Meshes[Index];
			if (Mesh)
			{
				// TODO proper bAdditionalViewFamily
				UStaticMeshThumbnailRenderer::Draw(Mesh, NewX, NewY, NewWidth, NewHeight, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
			}
		}
	});
}