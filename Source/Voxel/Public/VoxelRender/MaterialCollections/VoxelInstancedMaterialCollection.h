// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelCachedMaterialCollection.h"
#include "VoxelInstancedMaterialCollection.generated.h"

class UMaterialInstance;

UCLASS(Blueprintable, BlueprintType)
class VOXEL_API UVoxelInstancedMaterialCollectionTemplates : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
	UMaterialInterface* Template = nullptr;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generated Templates")
	UMaterialInterface* Template1x = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generated Templates")
	UMaterialInterface* Template2x = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generated Templates")
	UMaterialInterface* Template3x = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generated Templates")
	UMaterialInterface* Template4x = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generated Templates")
	UMaterialInterface* Template5x = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generated Templates")
	UMaterialInterface* Template6x = nullptr;

public:
#if WITH_EDITOR
	// Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End UObject Interface
#endif
};

USTRUCT(BlueprintType)
struct FVoxelInstancedMaterialCollectionLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	uint8 LayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInstance* LayerMaterialInstance = nullptr;

	inline bool operator==(int32 Other) const
	{
		return LayerIndex == Other;
	}
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelInstancedMaterialCollection : public UVoxelCachedMaterialCollection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 1, ClampMax = 6, UIMin = 1, UIMax = 6))
	int32 MaxMaterialsToBlendAtOnce = 6;

	// For example: If " Sides" is a redirect and there's a "Normal Sides" parameter not overriden, the value of the "Normal" parameter will be used instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FString> Redirects;

	// Parameters prefix in the template: eg, if VOXELPARAM_, VOXELPARAM_0:SomeParameterName will be set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", AdvancedDisplay)
	FString ParametersPrefix = "VOXELPARAM_";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
	UVoxelInstancedMaterialCollectionTemplates* Templates = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
	TArray<FVoxelInstancedMaterialCollectionLayer> Layers;
	
public:
	UVoxelInstancedMaterialCollection();
	
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual int32 GetMaxMaterialIndices() const override;
	virtual int32 GetMaterialIndex(FName Name) const override;
	virtual TArray<FMaterialInfo> GetMaterials() const override;
	virtual UMaterialInterface* GetIndexMaterial(uint8 Index) const override;
	//~ End UVoxelMaterialCollectionBase Interface

	//~ Begin UVoxelCachedMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const override;
	//~ End UVoxelCachedMaterialCollection Interface

#if WITH_EDITOR
	// Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End UObject Interface
#endif
};

// Use to share layers across multiple collections - eg to enable/disable tessellation
UCLASS(BlueprintType)
class VOXEL_API UVoxelInstancedMaterialCollectionInstance : public UVoxelInstancedMaterialCollection
{
	GENERATED_BODY()

public:
	// The layers will be copied from this collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	UVoxelInstancedMaterialCollection* LayersSource;

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual void InitializeCollection() override;
	//~ End UVoxelMaterialCollectionBase Interface
	
	// Begin UObject Interface
	virtual void PostLoad() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	// End UObject Interface
};