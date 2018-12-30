// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelMaterial.h"
#include "VoxelPaintMaterial.h"
#include "Logging/MessageLog.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelWorld.h"
#include "VoxelTools/VoxelTools.h"
#include "VoxelBPUtilities.generated.h"

#define VOXELINDEX_DISABLED(Name)  FMessageLog("PIE").Error(FText::FromString(FString(#Name) + TEXT(": Voxel index   is disabled (DISABLE_VOXELINDEX == ") + FString::FromInt(DISABLE_VOXELINDEX) + TEXT(")!")));
#define VOXELCOLORS_DISABLED(Name) FMessageLog("PIE").Error(FText::FromString(FString(#Name) + TEXT(": Voxel colors are disabled (ENABLE_VOXELCOLORS == ") + FString::FromInt(ENABLE_VOXELCOLORS) + TEXT(")!")));
#define VOXELACTORS_DISABLED(Name) FMessageLog("PIE").Error(FText::FromString(FString(#Name) + TEXT(": Voxel actors are disabled (ENABLE_VOXELACTORS == ") + FString::FromInt(ENABLE_VOXELACTORS) + TEXT(")!")));
#define VOXELGRASS_DISABLED(Name)  FMessageLog("PIE").Error(FText::FromString(FString(#Name) + TEXT(": Voxel grass is disabled (ENABLE_VOXELGRASS == ")    + FString::FromInt(ENABLE_VOXELGRASS)  + TEXT(")!")));

#if DISABLE_VOXELINDEX
#define CHECK_VOXELINDEX(Name) VOXELINDEX_DISABLED(Name)
#else
#define CHECK_VOXELINDEX(Name)
#endif

#if !ENABLE_VOXELACTORS
#define CHECK_VOXELCOLORS(Name) VOXELCOLORS_DISABLED(Name)
#else
#define CHECK_VOXELCOLORS(Name)
#endif

#if !ENABLE_VOXELACTORS
#define CHECK_VOXELACTORS(Name) VOXELACTORS_DISABLED(Name)
#else
#define CHECK_VOXELACTORS(Name)
#endif

#if !ENABLE_VOXELGRASS
#define CHECK_VOXELGRASS(Name) VOXELGRASS_DISABLED(Name)
#else
#define CHECK_VOXELGRASS(Name)
#endif

UCLASS()
class VOXEL_API UVoxelBPUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * FIntBox helpers
	 */
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox TranslateBox(const FIntBox& Box, const FIntVector& Position) { return Box.TranslateBy(Position); }

	// From -Radius(included) to Radius(excluded)
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox MakeBoxFromLocalPositionAndRadius(const FIntVector& Position, int Radius) { return FIntBox(FIntVector(-Radius), FIntVector(Radius)).TranslateBy(Position); }
	// From Floor(-Radius) (included) to Ceil(Radius) (excluded)
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static FIntBox MakeBoxFromGlobalPositionAndRadius(AVoxelWorld* World, const FVector& GlobalPosition, float RadiusInVoxels)
	{
		CHECK_WORLD_VOXELTOOLS(MakeBoxFromGlobalPositionAndRadius, FIntBox());
		return FIntBox(FIntVector(FMath::FloorToInt(-RadiusInVoxels)), FIntVector(FMath::CeilToInt(RadiusInVoxels))).TranslateBy(World->GlobalToLocal(GlobalPosition));
	}

public:
	/**
	 * FIntVector helpers
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector + IntVector", CompactNodeTitle = "+", Keywords = "+ add plus"), Category = "Math|IntVector")
	static FIntVector Add_IntVectorIntVector(FIntVector Left, FIntVector Right) { return Left + Right; }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector - IntVector", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|IntVector")
	static FIntVector Substract_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right) { return Left - Right; }
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector * IntVector", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right) { return FIntVector(Left.X * Right.X, Left.Y * Right.Y, Left.Z * Right.Z); }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector / int", CompactNodeTitle = "/", Keywords = "/ divide"), Category = "Math|IntVector")
	static FIntVector Divide_IntVectorInt(const FIntVector& Left, int Right) { return Left / Right; }
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector * int", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntVectorInt(const FIntVector& Left, int Right) { return Left * Right; }
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "int * IntVector", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntIntVector(int Left, const FIntVector& Right) { return Right * Left; }
		
public:
	/**
	 * FVoxelPaintMaterial helpers
	 */

	 // Create from color
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel", DisplayName = "Create RGB Paint Material")
	static FVoxelPaintMaterial CreateRGBPaintMaterial(FLinearColor Color, float Amount)
	{
		CHECK_VOXELCOLORS(CreateRGBPaintMaterial);

		FVoxelPaintMaterial Material(EVoxelPaintMaterialType::RGB);
		Material.Color = Color;
		Material.Amount = Amount;
		return Material;
	}

	// Create from index
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelPaintMaterial CreateIndexPaintMaterial(uint8 Index)
	{
		CHECK_VOXELINDEX(CreateIndexPaintMaterial);

		FVoxelPaintMaterial Material(EVoxelPaintMaterialType::Index);
		Material.Index = Index;
		return Material;
	}

	// Create from grass id
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelPaintMaterial CreateGrassPaintMaterial(uint8 GrassId)
	{
		CHECK_VOXELINDEX(CreateIndexPaintMaterial);

		FVoxelPaintMaterial Material(EVoxelPaintMaterialType::Grass);
		Material.Index = GrassId;
		return Material;
	}

	// Create from actor id
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelPaintMaterial CreateActorPaintMaterial(uint8 ActorId)
	{
		CHECK_VOXELINDEX(CreateIndexPaintMaterial);

		FVoxelPaintMaterial Material(EVoxelPaintMaterialType::Actor);
		Material.Index = ActorId;
		return Material;
	}

	// Apply a Paint Material to a Voxel Material
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial ApplyPaintMaterial(FVoxelMaterial Material, FVoxelPaintMaterial PaintMaterial)
	{
		PaintMaterial.ApplyToMaterial(Material);
		return Material;
	}

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FLinearColor GetColor(FVoxelMaterial Material)
	{
		CHECK_VOXELCOLORS(GetColor);
		return Material.GetLinearColor();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetColor(FVoxelMaterial Material, FLinearColor Color)
	{
		CHECK_VOXELCOLORS(SetColor);
		Material.SetColor(Color);
		return Material;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial CreateMaterialFromColor(FLinearColor Color)
	{
		CHECK_VOXELCOLORS(CreateMaterialFromColor);
		FVoxelMaterial Material;
		Material.SetColor(Color);
		return Material;
	}

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static uint8 GetIndex(FVoxelMaterial Material)
	{
		CHECK_VOXELINDEX(GetIndex);
		return Material.GetIndex();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetIndex(FVoxelMaterial Material, uint8 Index)
	{
		CHECK_VOXELINDEX(SetIndex);
		Material.SetIndex(Index);
		return Material;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial CreateMaterialFromIndex(uint8 Index)
	{
		CHECK_VOXELINDEX(CreateMaterialFromIndex);
		FVoxelMaterial Material;
		Material.SetIndex(Index);
		return Material;
	}

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static uint8 GetIndexA(FVoxelMaterial Material)
	{
		CHECK_VOXELCOLORS(GetIndexA);
		return Material.GetIndexA();
	}
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static uint8 GetIndexB(FVoxelMaterial Material)
	{
		CHECK_VOXELCOLORS(GetIndexB);
		return Material.GetIndexB();
	}
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static uint8 GetBlend(FVoxelMaterial Material)
	{
		CHECK_VOXELCOLORS(GetBlend);
		return Material.GetBlend();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetIndexA(FVoxelMaterial Material, uint8 Index)
	{
		CHECK_VOXELCOLORS(SetIndexA);
		Material.SetIndexA(Index);
		return Material;
	}
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetIndexB(FVoxelMaterial Material, uint8 Index)
	{
		CHECK_VOXELCOLORS(SetIndexB);
		Material.SetIndexB(Index);
		return Material;
	}
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetBlend(FVoxelMaterial Material, float Blend)
	{
		CHECK_VOXELCOLORS(SetBlend);
		Material.SetBlend(FMath::Clamp(FMath::RoundToInt(Blend * 255.999f), 0, 255));
		return Material;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial CreateMaterialFromDoubleIndex(uint8 IndexA, uint8 IndexB, float Blend)
	{
		CHECK_VOXELCOLORS(CreateMaterialFromDoubleIndex);
		FVoxelMaterial Material;
		SetIndexA(Material, IndexA);
		SetIndexB(Material, IndexB);
		SetBlend(Material, Blend);
		return Material;
	}
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static uint8 GetVoxelActorId(FVoxelMaterial Material)
	{
		CHECK_VOXELACTORS(GetVoxelActorId);
		return Material.GetVoxelActorId();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetVoxelActorId(FVoxelMaterial Material, uint8 VoxelSpawnedActorId)
	{
		CHECK_VOXELACTORS(SetVoxelActorId);
		Material.SetVoxelActorId(VoxelSpawnedActorId);
		return Material;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static uint8 GetVoxelGrassId(FVoxelMaterial Material)
	{
		CHECK_VOXELGRASS(GetVoxelGrassId);
		return Material.GetVoxelGrassId();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Voxel")
	static FVoxelMaterial SetVoxelGrassId(FVoxelMaterial Material, uint8 VoxelGrassId)
	{
		CHECK_VOXELGRASS(SetVoxelGrassId);
		Material.SetVoxelGrassId(VoxelGrassId);
		return Material;
	}
};
