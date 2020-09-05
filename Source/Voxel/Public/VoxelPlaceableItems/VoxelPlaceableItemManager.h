// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelPlaceableItemManager.generated.h"

struct FVoxelDataItem;

template<typename T>
class TVoxelDataItemWrapper;

class IVoxelWorldInterface;

class UVoxelWorldGenerator;
class UVoxelLineBatchComponent;

class FVoxelData;
class FVoxelWorldGeneratorCache;

USTRUCT(BlueprintType)
struct FVoxelDataItemConstructionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UVoxelWorldGenerator* Generator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelIntBox Bounds;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<float> Parameters;
};

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class VOXEL_API UVoxelPlaceableItemManager : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableDebug = true;
	
	// If true, will show all the data items bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (EditCondition = bEnableDebug))
	bool bDebugBounds = false;
	
public:
	// Do not call this directly: call the respective Add Data Item instead!
	UFUNCTION(BlueprintCallable, Category = "Voxel Placeable Item Manager")
	void AddDataItem(FVoxelDataItemConstructionInfo Info); // Important: parameter must not change names! See UK2Node_AddDataItem

	/**
	 * Draws a line in the world & in the voxel graph preview
	 * @param	Start		The start position in voxels
	 * @param	End			The end position in voxels
	 * @param	Color		The color
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel Placeable Item Manager")
	void DrawDebugLine(
		FVector Start, 
		FVector End,
		FLinearColor Color = FLinearColor::Red);
	
	/**
	 * Draws a point in the world & in the voxel graph preview
	 * @param	Position	The position in voxels
	 * @param	Color		The color
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel Placeable Item Manager")
	void DrawDebugPoint(
		FVector Position, 
		FLinearColor Color = FLinearColor::Green);

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel Placeable Item Manager")
	void OnGenerate();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel Placeable Item Manager")
	void OnClear();
	
	virtual void OnGenerate_Implementation() {}
	virtual void OnClear_Implementation() {}

public:
	using FVoxelDataItemPtr = TVoxelWeakPtr<const TVoxelDataItemWrapper<FVoxelDataItem>>;
	
	void Generate();
	void Clear();
	void ApplyToData(
		FVoxelData& Data, 
		FVoxelWorldGeneratorCache& Cache, 
		TMap<int32, FVoxelDataItemPtr>* OutItems = nullptr);

	const TArray<FVoxelDataItemConstructionInfo>& GetDataItemInfos() const { return DataItemInfos; }
	
private:
	UPROPERTY()
	TArray<FVoxelDataItemConstructionInfo> DataItemInfos;

public:
	struct FDebugLine
	{
		FVector Start;
		FVector End;
		FLinearColor Color;
	};
	struct FDebugPoint
	{
		FVector Position;
		FLinearColor Color;
	};

	const TArray<FDebugLine>& GetDebugLines() const { return DebugLines; }
	const TArray<FDebugPoint>& GetDebugPoints() const { return DebugPoints; }
	
	void DrawDebug(
		const IVoxelWorldInterface& VoxelWorldInterface, 
		UVoxelLineBatchComponent& LineBatchComponent);
	
private:
	TArray<FDebugLine> DebugLines;
	TArray<FDebugPoint> DebugPoints;
};