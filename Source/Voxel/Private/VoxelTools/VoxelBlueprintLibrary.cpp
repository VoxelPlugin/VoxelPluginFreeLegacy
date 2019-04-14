// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "IntBox.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelCacheManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "IVoxelPool.h"
#include "VoxelDefaultPools.h"

#include "Logging/MessageLog.h"
#include "Async/Async.h"

#define LOCTEXT_NAMESPACE "Voxel"

#define VOXELINDEX_DISABLED(Name)  VoxelLogBlueprintError(FText::FromString(FString(#Name) + TEXT(": Voxel index   is disabled (DISABLE_VOXELINDEX == ") + FString::FromInt(DISABLE_VOXELINDEX) + TEXT(")!")));
#define VOXELCOLORS_DISABLED(Name) VoxelLogBlueprintError(FText::FromString(FString(#Name) + TEXT(": Voxel colors are disabled (ENABLE_VOXELCOLORS == ") + FString::FromInt(ENABLE_VOXELCOLORS) + TEXT(")!")));

#if DISABLE_VOXELINDEX
#define CHECK_VOXELINDEX(Name) VOXELINDEX_DISABLED(Name)
#else
#define CHECK_VOXELINDEX(Name)
#endif

#if !ENABLE_VOXELCOLORS
#define CHECK_VOXELCOLORS(Name) VOXELCOLORS_DISABLED(Name)
#else
#define CHECK_VOXELCOLORS(Name)
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::ClearCache(AVoxelWorld* World, const TArray<FIntBox>& BoundsToKeepCached)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetCacheManager().ClearCache(BoundsToKeepCached);
}

void UVoxelBlueprintLibrary::ClearAllCache(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	ClearCache(World, {});
}

void UVoxelBlueprintLibrary::Cache(AVoxelWorld* World, const TArray<FIntBox>& BoundsToCache, bool bCacheValues, bool bCacheMaterials)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetCacheManager().Cache(BoundsToCache, bCacheValues, bCacheMaterials);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::Undo(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		VoxelLogBlueprintError(LOCTEXT("UndobEnableUndoRedo", "Undo: Undo called but bEnableUndoRedo is false. Please enable it on your voxel world"));
		return;
	}
	if (!Data.IsCurrentFrameEmpty())
	{
		VoxelLogBlueprintError(LOCTEXT("UndoSaveFrame", "Undo: Undo called but edits have been made since last SaveFrame. Please call SaveFrame after every edits"));
		return;
	}

	TArray<FIntBox> BoundsToUpdate;
	Data.Undo(BoundsToUpdate);
	World->GetLODManager().UpdateBounds(BoundsToUpdate);
}

void UVoxelBlueprintLibrary::Redo(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		VoxelLogBlueprintError(LOCTEXT("RedobEnableUndoRedo", "Redo: Redo called but bEnableUndoRedo is false. Please enable it on your voxel world"));
		return;
	}
	if (!Data.IsCurrentFrameEmpty())
	{
		VoxelLogBlueprintError(LOCTEXT("RedoSaveFrame", "Redo: Redo called but edits have been made since last SaveFrame. Please call SaveFrame after every edits"));
		return;
	}
	
	TArray<FIntBox> BoundsToUpdate;
	Data.Redo(BoundsToUpdate);
	World->GetLODManager().UpdateBounds(BoundsToUpdate);
}

void UVoxelBlueprintLibrary::SaveFrame(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		VoxelLogBlueprintError(LOCTEXT("SaveFrame", "SaveFrame: SaveFrame called but bEnableUndoRedo is false. Please enable it on your voxel world"));
		return;
	}
	Data.SaveFrame();
}

void UVoxelBlueprintLibrary::ClearFrames(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		VoxelLogBlueprintError(LOCTEXT("ClearFrames", "ClearFrames: ClearFrames called but bEnableUndoRedo is false. Please enable it on your voxel world"));
	}
	Data.ClearFrames();
}

///////////////////////////////////////////////////////////////////////////////

FVector UVoxelBlueprintLibrary::GetNormal(AVoxelWorld* World, const FIntVector& Position)
{
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();
	FVoxelReadScopeLock Lock(Data, FIntBox(Position - FIntVector(1), Position + FIntVector(2)), "GetNormal");
	return FVoxelDataUtilities::GetGradient(Data, Position, 0);
}

float UVoxelBlueprintLibrary::GetFloatOutput(AVoxelWorld* World, FName Name, int32 X, int32 Y, int32 Z)
{
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	const uint8* Index = Data.WorldGenerator->SingleFloatOutputsNames.Find(Name);
	if (!Index)
	{
		VoxelLogBlueprintError(FText::FromString("GetFloatOutput: No output named " + Name.ToString() + " and with type float found!"));
		return 0;
	}
	return Data.WorldGenerator->GetFloatOutput(X, Y, Z, *Index);
}

int32 UVoxelBlueprintLibrary::GetIntOutput(AVoxelWorld* World, FName Name, int32 X, int32 Y, int32 Z)
{
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	const uint8* Index = Data.WorldGenerator->SingleIntOutputsNames.Find(Name);
	if (!Index)
	{
		VoxelLogBlueprintError(FText::FromString("GetFloatOutput: No output named " + Name.ToString() + " and with type int found!"));
		return 0;
	}
	return Data.WorldGenerator->GetIntOutput(X, Y, Z, *Index);
}

FIntBox UVoxelBlueprintLibrary::GetBounds(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return World->GetData().WorldBounds;
}

void UVoxelBlueprintLibrary::BindDelegateToChunkGeneration(AVoxelWorld* World, FVoxelOnChunkGenerationDynamicDelegate Event)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	auto& Data = World->GetData();
	TWeakObjectPtr<AVoxelWorld> WorldPtr = World;

	Data.LockDelegatesForWrite();
	Data.PostGenerationDelegate.AddLambda([=](FVoxelStatsElement& Stats, FVoxelData& Data, const FIntBox& Bounds, const FVoxelChunk& Mesh)
	{
		AsyncTask(ENamedThreads::GameThread, [=]()
		{
			Event.ExecuteIfBound(WorldPtr.Get(), Bounds);
		});
	});
	Data.UnlockDelegatesForWrite();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::UpdatePosition(AVoxelWorld* World, const FIntVector& Position)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetLODManager().UpdatePosition(Position);
}

void UVoxelBlueprintLibrary::UpdateBounds(AVoxelWorld* World, const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetLODManager().UpdateBounds(Bounds, bWaitForAllChunksToFinishUpdating);
}

void UVoxelBlueprintLibrary::UpdateAll(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetLODManager().UpdateBounds(FIntBox::Infinite);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelBlueprintLibrary::GetTaskCount(AVoxelWorld* World)
{
	return World->IsCreated() ? FMath::Max(World->GetRenderer().GetTaskCount(), 0) : 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::CreateGlobalVoxelThreadPool(int32 MeshThreadCount)
{
	if (IsGlobalVoxelPoolCreated())
	{
		VoxelLogBlueprintError(LOCTEXT("CreateGlobalVoxelPool", "CreateGlobalVoxelPool called but global pool already created!"));
		return;
	}
	FVoxelDefaultPool::CreateGlobalPool(FMath::Max(1, MeshThreadCount));
}

void UVoxelBlueprintLibrary::DestroyGlobalVoxelThreadPool()
{
	if (!IsGlobalVoxelPoolCreated())
	{
		VoxelLogBlueprintError(LOCTEXT("DestroyGlobalVoxelPool", "DestroyGlobalVoxelPool called but global pool isn't created!"));
		return;
	}
	IVoxelPool::DestroyGlobalVoxelPool();
}

bool UVoxelBlueprintLibrary::IsGlobalVoxelPoolCreated()
{
	return IVoxelPool::IsGlobalVoxelPoolCreated();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntBox UVoxelBlueprintLibrary::MakeBoxFromGlobalPositionAndRadius(AVoxelWorld* World, const FVector& GlobalPosition, float RadiusInVoxels)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return FIntBox(FIntVector(FMath::FloorToInt(-RadiusInVoxels)), FIntVector(FMath::CeilToInt(RadiusInVoxels))).TranslateBy(World->GlobalToLocal(GlobalPosition));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntVector UVoxelBlueprintLibrary::Add_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right)
{
	return Left + Right;
}

FIntVector UVoxelBlueprintLibrary::Substract_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right)
{
	return Left - Right;
}

FIntVector UVoxelBlueprintLibrary::Multiply_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right)
{
	return FIntVector(Left.X * Right.X, Left.Y * Right.Y, Left.Z * Right.Z);
}

FIntVector UVoxelBlueprintLibrary::Divide_IntVectorInt(const FIntVector& Left, int32 Right)
{
	return Left / Right;
}

FIntVector UVoxelBlueprintLibrary::Multiply_IntVectorInt(const FIntVector& Left, int32 Right)
{
	return Left * Right;
}

FIntVector UVoxelBlueprintLibrary::Multiply_IntIntVector(int32 Left, const FIntVector& Right)
{
	return Right * Left;
}

int32 UVoxelBlueprintLibrary::GetMax_Intvector(const FIntVector& Vector)
{
	return Vector.GetMax();
}

int32 UVoxelBlueprintLibrary::GetMin_Intvector(const FIntVector& Vector)
{
	return Vector.GetMin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPaintMaterial UVoxelBlueprintLibrary::CreateRGBPaintMaterial(FLinearColor Color, float Amount, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA)
{
	CHECK_VOXELCOLORS(CreateRGBPaintMaterial);

	FVoxelPaintMaterial Material(EVoxelPaintMaterialType::RGBA);
	Material.Color.Color = Color;
	Material.Color.bPaintR = bPaintR;
	Material.Color.bPaintG = bPaintG;
	Material.Color.bPaintB = bPaintB;
	Material.Color.bPaintA = bPaintA;
	Material.Amount = Amount;
	return Material;
}

FVoxelPaintMaterial UVoxelBlueprintLibrary::CreateIndexPaintMaterial(uint8 Index)
{
	CHECK_VOXELINDEX(CreateIndexPaintMaterial);

	FVoxelPaintMaterial Material(EVoxelPaintMaterialType::Index);
	Material.Index = Index;
	return Material;
}

FVoxelPaintMaterial UVoxelBlueprintLibrary::CreateDoubleIndexSetPaintMaterial(uint8 IndexA, uint8 IndexB, float Blend, bool bSetIndexA, bool bSetIndexB, bool bSetBlend)
{
	CHECK_VOXELINDEX(CreateIndexPaintMaterial);

	FVoxelPaintMaterial Material(EVoxelPaintMaterialType::DoubleIndexSet);
	Material.DoubleIndex.IndexA = IndexA;
	Material.DoubleIndex.IndexB = IndexB;
	Material.DoubleIndex.Blend = Blend;
	Material.DoubleIndex.bSetIndexA = bSetIndexA;
	Material.DoubleIndex.bSetIndexB = bSetIndexB;
	Material.DoubleIndex.bSetBlend = bSetBlend;
	return Material;
}

FVoxelPaintMaterial UVoxelBlueprintLibrary::CreateDoubleIndexBlendPaintMaterial(uint8 Index, float Amount)
{
	CHECK_VOXELINDEX(CreateIndexPaintMaterial);

	FVoxelPaintMaterial Material(EVoxelPaintMaterialType::DoubleIndexBlend);
	Material.Index = Index;
	Material.Amount = Amount;
	return Material;
}

FVoxelMaterial UVoxelBlueprintLibrary::ApplyPaintMaterial(FVoxelMaterial Material, FVoxelPaintMaterial PaintMaterial)
{
	PaintMaterial.ApplyToMaterial(Material);
	return Material;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor UVoxelBlueprintLibrary::GetColor(FVoxelMaterial Material)
{
	CHECK_VOXELCOLORS(GetColor);
	return Material.GetLinearColor();
}

FVoxelMaterial UVoxelBlueprintLibrary::SetColor(FVoxelMaterial Material, FLinearColor Color)
{
	CHECK_VOXELCOLORS(SetColor);
	Material.SetColor(Color);
	return Material;
}

FVoxelMaterial UVoxelBlueprintLibrary::CreateMaterialFromColor(FLinearColor Color)
{
	CHECK_VOXELCOLORS(CreateMaterialFromColor);
	FVoxelMaterial Material;
	Material.SetColor(Color);
	return Material;
}

///////////////////////////////////////////////////////////////////////////////

uint8 UVoxelBlueprintLibrary::GetIndex(FVoxelMaterial Material)
{
	CHECK_VOXELINDEX(GetIndex);
	return Material.GetIndex();
}

FVoxelMaterial UVoxelBlueprintLibrary::SetIndex(FVoxelMaterial Material, uint8 Index)
{
	CHECK_VOXELINDEX(SetIndex);
	Material.SetIndex(Index);
	return Material;
}

FVoxelMaterial UVoxelBlueprintLibrary::CreateMaterialFromIndex(uint8 Index)
{
	CHECK_VOXELINDEX(CreateMaterialFromIndex);
	FVoxelMaterial Material;
	Material.SetIndex(Index);
	return Material;
}

///////////////////////////////////////////////////////////////////////////////

uint8 UVoxelBlueprintLibrary::GetIndexA(FVoxelMaterial Material)
{
	CHECK_VOXELCOLORS(GetIndexA);
	return Material.GetIndexA();
}

uint8 UVoxelBlueprintLibrary::GetIndexB(FVoxelMaterial Material)
{
	CHECK_VOXELCOLORS(GetIndexB);
	return Material.GetIndexB();
}

uint8 UVoxelBlueprintLibrary::GetBlend(FVoxelMaterial Material)
{
	CHECK_VOXELCOLORS(GetBlend);
	return Material.GetBlend();
}

FVoxelMaterial UVoxelBlueprintLibrary::SetIndexA(FVoxelMaterial Material, uint8 Index)
{
	CHECK_VOXELCOLORS(SetIndexA);
	Material.SetIndexA(Index);
	return Material;
}

FVoxelMaterial UVoxelBlueprintLibrary::SetIndexB(FVoxelMaterial Material, uint8 Index)
{
	CHECK_VOXELCOLORS(SetIndexB);
	Material.SetIndexB(Index);
	return Material;
}

FVoxelMaterial UVoxelBlueprintLibrary::SetBlend(FVoxelMaterial Material, float Blend)
{
	CHECK_VOXELCOLORS(SetBlend);
	Material.SetBlend(FMath::Clamp(FMath::RoundToInt(Blend * 255.999f), 0, 255));
	return Material;
}

FVoxelMaterial UVoxelBlueprintLibrary::CreateMaterialFromDoubleIndex(uint8 IndexA, uint8 IndexB, float Blend)
{
	CHECK_VOXELCOLORS(CreateMaterialFromDoubleIndex);
	FVoxelMaterial Material;
	SetIndexA(Material, IndexA);
	SetIndexB(Material, IndexB);
	SetBlend(Material, Blend);
	return Material;
}
#undef LOCTEXT_NAMESPACE