// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelIntBox.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTestLibrary.generated.h"

class AVoxelWorld;

USTRUCT(BlueprintType)
struct FVoxelTestValues
{
	GENERATED_BODY()

	TSharedRef<const TArray<FVoxelValue>> Values = MakeShared<TArray<FVoxelValue>>();
};

UCLASS()
class VOXEL_API UVoxelTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Test", meta = (DefaultToSelf = "World"))
	static FVoxelTestValues ReadValues(AVoxelWorld* World, FVoxelIntBox Bounds);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Test", meta = (DefaultToSelf = "World"))
	static void TestValues(FVoxelTestValues ValuesA, FVoxelTestValues ValuesB);
};