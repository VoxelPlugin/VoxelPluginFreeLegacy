// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelGenerator.generated.h"

class FVoxelGeneratorInstance;
class FVoxelTransformableGeneratorInstance;
struct FVoxelGeneratorParameter;

/**
 * A UVoxelGenerator is used to create a FVoxelGeneratorInstance
 */
UCLASS(BlueprintType, Abstract)
class VOXEL_API UVoxelGenerator : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGenerator Interface
	virtual void ApplyParameters(const TMap<FName, FString>& Parameters);
	virtual void GetParameters(TArray<FVoxelGeneratorParameter>& OutParameters) const;
	
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance(const TMap<FName, FString>& Parameters);
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance();
	//~ End UVoxelGenerator Interface

protected:
	TMap<FName, FString> ApplyParametersInternal(const TMap<FName, FString>& Parameters);
};

// Generator that can be moved around
UCLASS(Abstract)
class VOXEL_API UVoxelTransformableGenerator : public UVoxelGenerator
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelTransformableGenerator Interface
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance(const TMap<FName, FString>& Parameters);
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance();
	//~ End UVoxelTransformableGenerator Interface
	
	//~ Begin UVoxelGenerator Interface
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance(const TMap<FName, FString>& Parameters) override;
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	//~ End UVoxelGenerator Interface
};

UCLASS(Abstract)
class VOXEL_API UVoxelTransformableGeneratorWithBounds : public UVoxelTransformableGenerator
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelTransformableGeneratorWithBounds Interface
	virtual FVoxelIntBox GetBounds() const;
	//~ End UVoxelTransformableGeneratorWithBounds Interface
};