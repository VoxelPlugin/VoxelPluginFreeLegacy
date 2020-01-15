// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterialCollection.generated.h"

struct FVoxelBlendedMaterialSorted;
struct FVoxelBlendedMaterialUnsorted;
class UTexture;
class UMaterialInstanceDynamic;
class UPhysicalMaterial;
class UMaterialInstanceConstant;
class UMaterialFunction;
class UMaterialInterface;
class UMaterial;

UCLASS(Abstract)
class VOXEL_API UVoxelMaterialCollectionBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bShouldGenerateBlendings = false;
	
	//~ Begin UVoxelMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index, bool bTessellation) const;
	virtual TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted> GetBlendedMaterialsMap() const;
	//~ End UVoxelMaterialCollection Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType, Blueprintable)
struct FVoxelBasicMaterialCollectionElement
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UMaterialInterface* Material = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UMaterialInterface* TessellatedMaterial = nullptr;
};

// Material collection that do not generate any blending
UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelBasicMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FVoxelBasicMaterialCollectionElement> Materials;
	
	//~ Begin UVoxelMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index, bool bTessellation) const override;
	//~ End UVoxelMaterialCollection Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMaterialCollectionDoubleIndex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 I = 0;
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 J = 0;

	FVoxelMaterialCollectionDoubleIndex() {}
	FVoxelMaterialCollectionDoubleIndex(int32 I, int32 J)
		: I(I)
		, J(J)
	{
	}

	inline bool operator==(const FVoxelMaterialCollectionDoubleIndex& Other) const
	{
		return I == Other.I && J == Other.J;
	}
};

inline uint32 GetTypeHash(const FVoxelMaterialCollectionDoubleIndex& O)
{
	return O.I + 11311 * O.J;
}

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMaterialCollectionTripleIndex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 I = 0;
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 J = 0;
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 K = 0;

	FVoxelMaterialCollectionTripleIndex() {}
	FVoxelMaterialCollectionTripleIndex(int32 I, int32 J, int32 K)
		: I(I)
		, J(J)
		, K(K)
	{
	}

	inline bool operator==(const FVoxelMaterialCollectionTripleIndex& Other) const
	{
		return I == Other.I && J == Other.J && K == Other.K;
	}
};

inline uint32 GetTypeHash(const FVoxelMaterialCollectionTripleIndex& O)
{
	return O.I + 11311 * O.J + 16453 * O.K;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelMaterialCollectionSingleRefHolder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	TMap<uint8, UMaterialInterface*> Map;
};

UCLASS()
class VOXEL_API UVoxelMaterialCollectionDoubleRefHolder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	TMap<FVoxelMaterialCollectionDoubleIndex, UMaterialInterface*> Map;

	// Sorted index to unsorted
	UPROPERTY()
	TMap<FVoxelMaterialCollectionDoubleIndex, FVoxelMaterialCollectionDoubleIndex> SortedIndexMap;
};

UCLASS()
class VOXEL_API UVoxelMaterialCollectionTripleRefHolder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	TMap<FVoxelMaterialCollectionTripleIndex, UMaterialInterface*> Map;
	
	// Sorted index to unsorted
	UPROPERTY()
	TMap<FVoxelMaterialCollectionTripleIndex, FVoxelMaterialCollectionTripleIndex> SortedIndexMap;
};

///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXEL_API FVoxelMaterialCollectionGenerated
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UVoxelMaterialCollectionSingleRefHolder* GeneratedSingleMaterials = nullptr;

	UPROPERTY()
	UVoxelMaterialCollectionDoubleRefHolder* GeneratedDoubleMaterials = nullptr;

	UPROPERTY()
	UVoxelMaterialCollectionTripleRefHolder* GeneratedTripleMaterials = nullptr;

public:
	UMaterialInterface* GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index) const;
};

///////////////////////////////////////////////////////////////////////////////

// Generate materials to blend between provided materials
UCLASS(BlueprintType)
class VOXEL_API UVoxelMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()
		
public:
	UVoxelMaterialCollection();
	
public:
	UPROPERTY(EditAnywhere, Category = "General")
	bool bEnableTessellation = false;
	
	UPROPERTY(EditAnywhere, Category = "General")
	bool bHidePhysicalMaterials = false;

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

	UPROPERTY()
	FVoxelMaterialCollectionGenerated GeneratedMaterials;

	UPROPERTY()
	FVoxelMaterialCollectionGenerated GeneratedMaterialsTess;

public:
	//~ Begin UVoxelMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelBlendedMaterialUnsorted& Index, bool bTessellation) const override;
	virtual TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted> GetBlendedMaterialsMap() const override;
	//~ End UVoxelMaterialCollection Interface

public:
	bool IsValidIndex(uint8 Index) const;
	// Name is either the name of a material function or a material instance
	int32 GetMaterialIndex(FName Name) const;
};