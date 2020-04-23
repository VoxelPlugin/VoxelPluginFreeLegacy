// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterialCollectionBase.h"
#include "VoxelMaterialCollection.generated.h"

class UMaterial;
class UMaterialFunction;
class UMaterialInstanceDynamic;
class UMaterialInstanceConstant;
class UPhysicalMaterial;

USTRUCT()
struct FVoxelMaterialCollectionElementIndex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Index"))
	uint8 InstanceIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UMaterialInstanceConstant* MaterialInstance = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UPhysicalMaterial* PhysicalMaterial = nullptr;
};

USTRUCT()
struct FVoxelMaterialCollectionElement
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, Category = "Voxel")
	uint8 Index = 0;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UMaterialFunction* MaterialFunction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UPhysicalMaterial* PhysicalMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Instances"))
	TArray<FVoxelMaterialCollectionElementIndex> Children;
};

UCLASS(Deprecated)
class VOXEL_API UDEPRECATED_VoxelMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()
		
public:
	UPROPERTY(EditAnywhere, Category = "General")
	bool bEnableTessellation = true;
	
	UPROPERTY(EditAnywhere, Category = "General")
	bool bHidePhysicalMaterials = true;

public:
	UPROPERTY(EditAnywhere, Category = "Templates")
	UMaterial* SingleMaterialTemplate;

	UPROPERTY(EditAnywhere, Category = "Templates")
	UMaterial* DoubleMaterialTemplate;

	UPROPERTY(EditAnywhere, Category = "Templates")
	UMaterial* TripleMaterialTemplate;

	// The parameters of this instance will be copied to the templates
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Templates")
	UMaterialInstanceConstant* TemplateInstanceParameters;

public:
	UPROPERTY(EditAnywhere, Category = "Layers")
	TArray<FVoxelMaterialCollectionElement> Materials;

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual int32 GetMaxMaterialIndices() const override;
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const override;
	// Name is either the name of a material function or a material instance
	virtual int32 GetMaterialIndex(FName Name) const override;
	//~ End UVoxelMaterialCollectionBase Interface
};