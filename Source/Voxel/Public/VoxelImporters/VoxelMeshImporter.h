// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelMeshImporter.generated.h"

class UTexture;
class UTextureRenderTarget2D;
class UVoxelDataAsset;
class UStaticMesh;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;
struct FVoxelDataAssetData;

struct FVoxelMeshImporterInputData
{
	TArray<FVector> Vertices;
	TArray<FIntVector> Triangles;
	TArray<FVector2D> UVs;
};

// We don't want to copy the arrays in the BP, so use an object for that
UCLASS(BlueprintType)
class VOXEL_API UVoxelMeshImporterInputData : public UObject
{
	GENERATED_BODY()

public:
	FVoxelMeshImporterInputData Data;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMeshImporterRenderTargetCache
{
	GENERATED_BODY()

	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UTextureRenderTarget2D* ColorsRenderTarget = nullptr;

	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UTextureRenderTarget2D* UVsRenderTarget = nullptr;
	
	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UMaterialInterface* LastRenderedColorsMaterial = nullptr;
	
	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	UMaterialInterface* LastRenderedUVsMaterial = nullptr;
	
	UPROPERTY(Category = "Cache", BlueprintReadOnly, VisibleAnywhere, Transient)
	int32 LastRenderedRenderTargetSize = 0;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMeshImporterSettings
{
	GENERATED_BODY()

	FVoxelMeshImporterSettings();
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0"))
	float VoxelSize = 100;
	
	// Will sample ColorsMaterial at the mesh UVs to get the voxel colors
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	bool bPaintColors = true;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bPaintColors"))
	UMaterialInterface* ColorsMaterial = nullptr;

	// Will sample UVChannelsMaterial at the mesh UVs to get the voxel UVs
	// RG will go in first UV channel, BA in second
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere)
	bool bPaintUVs = true;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (EditCondition = bPaintUVs))
	UMaterialInterface* UVsMaterial = nullptr;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (InlineEditConditionToggle))
	bool bSetSingleIndex = false;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bSetSingleIndex"))
	uint8 SingleIndex = 0;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (InlineEditConditionToggle))
	bool bSetDoubleIndex = false;
	
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bSetDoubleIndex"))
	uint8 DoubleIndex = 0;

	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	int32 RenderTargetSize = 4096;
	
	// If true, will hide leaks by having holes instead
	// If false, leaks will be long tubes going through the entire asset
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	bool bHideLeaks = true;

	// Adds an offset to the distance field: a negative offset will make the volume bigger, a positive one will make it smaller.
	// eg try with -0.5 or 0.5
	// Might need to increase MaxVoxelDistanceFromTriangle if you go beyond -1
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	float DistanceFieldOffset = 0;

	// Distance will be exact for voxels under this distance from a triangle
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay, meta = (ClampMin = "1"))
	int32 MaxVoxelDistanceFromTriangle = 1;

	// Increase this if the shadows/normals quality is bad. Might require to increase MaxVoxelDistanceFromTriangle
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	float DistanceDivisor = 1;

	// If true, will compute the distance of every voxel
	// If false, just set voxels inside the mesh to -1 and voxels outside to 1, 
	// with voxels under a distance of MaxVoxelDistanceFromTriangle from the mesh having the exact distance
	// Much faster if false
	UPROPERTY(Category = "Import Configuration", BlueprintReadWrite, EditAnywhere, AdvancedDisplay)
	bool bComputeExactDistance = false;
};

UCLASS()
class VOXEL_API UVoxelMeshImporterLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh, FVoxelMeshImporterInputData& Data);
	
	static bool ConvertMeshToVoxels(
		UObject* WorldContextObject,
		const FVoxelMeshImporterInputData& Mesh,
		const FTransform& Transform,
		const FVoxelMeshImporterSettings& Settings,
		FVoxelMeshImporterRenderTargetCache& RenderTargetCache,
		FVoxelDataAssetData& OutAsset,
		FIntVector& OutOffset,
		int32& OutNumLeaks);

	static void ConvertMeshToDistanceField(
		const FVoxelMeshImporterInputData& Mesh,
		const FTransform& Transform,
		float VoxelSize,
		float MaxDistance,
		// Needed if we want a smooth import
		float BoxExtension,
		TArray<float>& OutDistanceField,
		FIntVector& OutSize,
		FIntVector& OutOffset,
		int32& OutNumLeaks);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer")
	static UVoxelMeshImporterInputData* CreateMeshDataFromStaticMesh(UStaticMesh* StaticMesh);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer", meta = (WorldContext = "WorldContextObject"))
	static UTextureRenderTarget2D* CreateTextureFromMaterial(
		UObject* WorldContextObject,
		UMaterialInterface* Material, 
		int32 Width = 1024, 
		int32 Height = 1024);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Mesh Importer", meta = (WorldContext = "WorldContextObject"))
	static void ConvertMeshToVoxels(
		UObject* WorldContextObject,
		UVoxelMeshImporterInputData* Mesh,
		FTransform Transform,
		bool bSubtractive,
		FVoxelMeshImporterSettings Settings,
		UPARAM(ref) FVoxelMeshImporterRenderTargetCache& RenderTargetCache,
		UVoxelDataAsset*& Asset,
		int32& NumLeaks);
};

/**
 * Actor that creates a VoxelDataAsset from a static mesh
 */
UCLASS(NotBlueprintType, NotBlueprintable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HOLD", "LOD", "Cooking"))
class VOXEL_API AVoxelMeshImporter : public AActor
{
	GENERATED_BODY()

public:
	// The static mesh to import from
	UPROPERTY(EditAnywhere, Category = "Import Configuration")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Import Configuration", meta = (ShowOnlyInnerProperties))
	FVoxelMeshImporterSettings Settings;

	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint32 SizeX;

	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint32 SizeY;
	
	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint32 SizeZ;
	
	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	uint64 NumberOfVoxels;

	UPROPERTY(VisibleAnywhere, Category = "Expected Size")
	float SizeInMB;

	AVoxelMeshImporter();

protected:
	virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif

private:
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY(Transient)
	FBox CachedBox;

	UPROPERTY(Transient)
	UStaticMesh* CachedStaticMesh;

	UPROPERTY(Transient)
	TArray<FVector> CachedVertices;
	
	UPROPERTY(Transient)
	FTransform CachedTransform;

	void InitMaterialInstance();
	void UpdateSizes();
};